// FOUNDRY LINUX — foundrylinux.org

function App() {
  return (
    <>
      <Topbar />
      <main>
        <Hero />
        <Kit />
        <Install />
        <Editions />
      </main>
      <Foot />
    </>
  );
}

ReactDOM.createRoot(document.getElementById("root")).render(<App />);
