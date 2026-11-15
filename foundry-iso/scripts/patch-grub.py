#!/usr/bin/env python3
"""Prepare the live-build GRUB menu before the single EFI/menu ISO commit."""
import pathlib
import re
import sys


def patch(content):
    content = content.replace('set default=0\n',
        'set default=0\nset timeout=5\nset gfxmode=auto\nset gfxpayload=keep\n', 1)
    content = content.replace('menuentry "Debian GNU/Linux - live" {',
                              'menuentry "Foundry Linux - Live" {')
    content = content.replace('menuentry "Debian GNU/Linux - live (fail-safe mode)" {',
                              'menuentry "Foundry Linux - Live (safe mode)" {')
    content = content.replace('menuentry "Debian GNU/Linux - live, kernel',
                              'menuentry "Foundry Linux - Live, kernel')
    linux = re.search(r'^(linux\s+.+)$', content, re.M)
    initrd = re.search(r'^(initrd\s+.+)$', content, re.M)
    menu = re.search(r'^menuentry[^\n]*\{\n.*?^}', content, re.M | re.S)
    if not all([linux, initrd, menu]):
        raise ValueError('Expected live-build menuentry, linux and initrd lines')
    entry = ('\nmenuentry "Install Foundry Linux" {\n' + linux.group(1) +
             ' automatic-calamares\n' + initrd.group(1) + '\n}\n')
    content = content[:menu.end()] + entry + content[menu.end():]
    return re.sub(r'^(?:insmod tga|background_image ).*\n', '', content, flags=re.M)


if __name__ == '__main__':
    path = pathlib.Path(sys.argv[1])
    path.write_text(patch(path.read_text()))
