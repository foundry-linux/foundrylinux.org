#!/usr/bin/env python3
"""Launch only the temporary spike resources; persist every created ID.

No credentials are written to the receipt or user-data. AWS CLI authentication
stays on the operator machine; the instance receives only an SSM role.
"""
import argparse
import base64
import datetime as dt
import gzip
import hashlib
import io
import json
import pathlib
import subprocess
import tarfile
import time

parser = argparse.ArgumentParser()
parser.add_argument("--profile", required=True)
parser.add_argument("--state", type=pathlib.Path, required=True)
parser.add_argument("--subnet", required=True)
parser.add_argument("--vpc", required=True)
parser.add_argument("--ami", required=True)
parser.add_argument("--stacks-repo", type=pathlib.Path, required=True)
parser.add_argument("--instance-type", default="m7i.xlarge")
parser.add_argument("--root-gib", type=int, required=True)
parser.add_argument("--build-gib", type=int, required=True)
parser.add_argument("--prepare-only", action="store_true")
args = parser.parse_args()
if args.root_gib < 8 or args.build_gib < 8 or args.root_gib == args.build_gib:
    parser.error("Use distinct root/build capacities of at least 8 GiB for unambiguous device selection")
region = "ap-south-1"
root = pathlib.Path(__file__).resolve().parents[2]
now = dt.datetime.now(dt.timezone.utc)
name = "foundry-spike-" + now.strftime("%Y%m%d-%H%M%S")
state = {"name": name, "region": region, "profile": args.profile,
         "created_at": now.isoformat(), "retention": "KEEP disks and caches; STOP instance"}
args.state.parent.mkdir(parents=True, exist_ok=True, mode=0o700)
if args.state.exists():
    raise SystemExit("Receipt already exists; inspect/resume existing resources instead of duplicating them")


def save():
    args.state.write_text(json.dumps(state, indent=2) + "\n")
    args.state.chmod(0o600)


def aws(service, action, **kwargs):
    command = ["aws", "--profile", args.profile, "--region", region,
               "--no-cli-pager", "--output", "json", service, action]
    if kwargs:
        command += ["--cli-input-json", json.dumps(kwargs)]
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode:
        raise RuntimeError(f"{service} {action}: {result.stderr.strip()}")
    return json.loads(result.stdout) if result.stdout.strip() else {}


def git(repo, *parts):
    return subprocess.check_output(["git", "-C", str(repo), *parts])


source_commit = git(root, "rev-parse", "HEAD").decode().strip()
stacks_commit = git(args.stacks_repo, "rev-parse", "HEAD").decode().strip()
files = {
    "run.sh": (root / "scripts/aws-spike/run.sh").read_bytes(),
    "bootstrap.sh": (root / "scripts/aws-spike/bootstrap.sh").read_bytes(),
    "config.sh": f"SOURCE_COMMIT={source_commit}\nBUILD_GIB={args.build_gib}\n".encode(),
    "stacks-source.txt": f"{stacks_commit}\n".encode(),
}
for filename in ["stacks/stacks.lock.json", "stacks/stack-base-ubuntu/Dockerfile",
                 "stacks/stack-build-ubuntu/Dockerfile", "stacks/stack-deb/Dockerfile"]:
    files[filename] = git(args.stacks_repo, "show", f"{stacks_commit}:{filename}")
for filename in ["observe.py", "cache.py", "vm.py", "audit-packages.py"]:
    files[filename] = (root / "scripts/aws-spike" / filename).read_bytes()
for patch in sorted((root / "scripts/aws-spike/patches").glob("*.patch")):
    files["patches/" + patch.name] = patch.read_bytes()
state["payload_sha256"] = {name: hashlib.sha256(data).hexdigest() for name, data in files.items()}
archive = io.BytesIO()
with tarfile.open(fileobj=archive, mode="w:gz") as tar:
    for filename, content in files.items():
        info = tarfile.TarInfo(filename)
        info.size = len(content)
        info.mode = 0o644
        tar.addfile(info, io.BytesIO(content))
payload = base64.b64encode(archive.getvalue()).decode()
userdata = gzip.compress(f"""#!/bin/bash
set -euo pipefail
mkdir -p /opt/foundry-spike
echo '{payload}' | base64 -d | tar -xz -C /opt/foundry-spike
shutdown -h +480 'Foundry spike runtime guard; retain disks'
bash /opt/foundry-spike/bootstrap.sh
""".encode())
assert len(userdata) <= 16384, "EC2 user-data exceeds its size limit"
state.update(source_commit=source_commit, stacks_commit=stacks_commit,
             instance_type=args.instance_type, ami=args.ami, subnet=args.subnet,
             root_gib=args.root_gib, build_gib=args.build_gib)
if args.prepare_only:
    print(json.dumps({**state, "compressed_userdata_bytes": len(userdata)}, indent=2))
    raise SystemExit(0)

identity = aws("sts", "get-caller-identity")
state["account"] = identity["Account"]
save()
role = name + "-ssm"
trust = {"Version": "2012-10-17", "Statement": [{"Effect": "Allow", "Principal": {
    "Service": "ec2.amazonaws.com"}, "Action": "sts:AssumeRole"}]}
try:
    aws("iam", "create-role", RoleName=role, AssumeRolePolicyDocument=json.dumps(trust),
        Tags=[{"Key": "Project", "Value": "foundry-spike"}])
    state["instance_role"] = role
    save()
    aws("iam", "attach-role-policy", RoleName=role,
        PolicyArn="arn:aws:iam::aws:policy/AmazonSSMManagedInstanceCore")
    aws("iam", "create-instance-profile", InstanceProfileName=role)
    state["instance_profile"] = role
    save()
    aws("iam", "add-role-to-instance-profile", InstanceProfileName=role, RoleName=role)
    group = aws("ec2", "create-security-group", GroupName=name,
        Description="Temporary Foundry spike; no inbound access; SSM administration", VpcId=args.vpc,
        TagSpecifications=[{"ResourceType": "security-group", "Tags": [{"Key": "Project", "Value": "foundry-spike"}]}])
    state["security_group"] = group["GroupId"]
    save()
    request = dict(ImageId=args.ami, InstanceType=args.instance_type, MinCount=1, MaxCount=1,
        ClientToken=name, IamInstanceProfile={"Name": role},
        NetworkInterfaces=[{"DeviceIndex": 0, "SubnetId": args.subnet,
                            "Groups": [group["GroupId"]], "AssociatePublicIpAddress": True}],
        CpuOptions={"NestedVirtualization": "enabled"},
        MetadataOptions={"HttpTokens": "required", "HttpEndpoint": "enabled", "HttpPutResponseHopLimit": 1},
        InstanceInitiatedShutdownBehavior="stop", UserData=base64.b64encode(userdata).decode(),
        BlockDeviceMappings=[{"DeviceName": "/dev/sda1", "Ebs": {"VolumeSize": args.root_gib,
            "VolumeType": "gp3", "Encrypted": True, "DeleteOnTermination": False}},
            {"DeviceName": "/dev/sdf", "Ebs": {"VolumeSize": args.build_gib, "VolumeType": "gp3",
             "Encrypted": True, "DeleteOnTermination": False}}],
        TagSpecifications=[{"ResourceType": kind, "Tags": [{"Key": "Name", "Value": name},
            {"Key": "Project", "Value": "foundry-spike"}, {"Key": "Retention", "Value": "keep-for-warm-run"}]}
            for kind in ["instance", "volume"]])
    for attempt in range(12):
        try:
            instance = aws("ec2", "run-instances", **request)["Instances"][0]
            break
        except RuntimeError as error:
            if "Invalid IAM Instance Profile" not in str(error) or attempt == 11:
                raise
            time.sleep(5)
    state["instance_id"] = instance["InstanceId"]
    save()
    stop_role = name + "-stop"
    trust["Statement"][0]["Principal"]["Service"] = "scheduler.amazonaws.com"
    aws("iam", "create-role", RoleName=stop_role, AssumeRolePolicyDocument=json.dumps(trust))
    state["stop_role"] = stop_role
    save()
    stop_policy = {"Version": "2012-10-17", "Statement": [{"Effect": "Allow",
        "Action": "ec2:StopInstances", "Resource": f"arn:aws:ec2:{region}:{state['account']}:instance/{state['instance_id']}"}]}
    aws("iam", "put-role-policy", RoleName=stop_role, PolicyName="StopOnlySpike",
        PolicyDocument=json.dumps(stop_policy))
    deadline = dt.datetime.now(dt.timezone.utc) + dt.timedelta(hours=8)
    for attempt in range(12):
        try:
            aws("scheduler", "create-schedule", Name=name, ScheduleExpression=f"at({deadline.strftime('%Y-%m-%dT%H:%M:%S')})",
                ScheduleExpressionTimezone="UTC", FlexibleTimeWindow={"Mode": "OFF"},
                ActionAfterCompletion="DELETE", Target={"Arn": "arn:aws:scheduler:::aws-sdk:ec2:stopInstances",
                    "RoleArn": f"arn:aws:iam::{state['account']}:role/{stop_role}",
                    "Input": json.dumps({"InstanceIds": [state["instance_id"]]}),
                    "RetryPolicy": {"MaximumRetryAttempts": 3, "MaximumEventAgeInSeconds": 3600}})
            break
        except RuntimeError as error:
            if "assume" not in str(error).lower() or attempt == 11:
                raise
            time.sleep(5)
    state["deadline_schedule"] = name
    state["deadline_utc"] = deadline.isoformat()
    save()
    print(json.dumps(state, indent=2))
except BaseException as error:
    state["launch_error"] = str(error)
    save()
    if state.get("instance_id"):
        aws("ec2", "stop-instances", InstanceIds=[state["instance_id"]])
    raise
