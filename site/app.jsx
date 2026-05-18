// FOUNDRY LINUX — foundrylinux.org
import { Topbar, Hero, Kit, Install, Editions, Foot } from './sections.jsx';

export function App() {
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
