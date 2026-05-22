// FOUNDRY LINUX — foundrylinux.org
import { Topbar, Hero, Forge, Install, Editions, Docs, Foot } from './sections.jsx';

export function App() {
  return (
    <>
      <Topbar />
      <main>
        <Hero />
        <Forge />
        <Install />
        <Editions />
        <Docs />
      </main>
      <Foot />
    </>
  );
}
