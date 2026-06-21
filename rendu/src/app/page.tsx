import { GiRadioTower } from "react-icons/gi";
import Link from "next/link";

export default function Home() {
  return (
    <main className="flex min-h-screen flex-col gap-5 items-center justify-center bg-teal-950 text-white px-4">
      {/* Logo */}
      <div className="mx-auto w-24 h-24 bg-teal-500 rounded-2xl flex items-center justify-center shadow-md shadow-teal-500 hover:scale-101">
        <GiRadioTower size={82} />
      </div>

      <h1 className="text-5xl font-bold tracking-tight">
        IoT<span className="text-teal-400">Flow</span>
      </h1>
      <div className="max-w-2xl items-center justify-center text-center">
        <p className="text-lg text-gray-300 leading-relaxed">
          Pilotez vos objets connectés en temps réel, visualisez vos données et
          automatisez vos dispositifs depuis une interface moderne et réactive.
        </p>
      </div>

      <Link
        href="/dashboard"
        className="px-8 py-4 bg-teal-500 hover:bg-teal-400 text-gray-950 font-semibold rounded-xl transition-all shadow-lg shadow-teal-500/30 hover:shadow-teal-400/40 hover:scale-105"
      >
        Accéder au Dashboard
      </Link>

      {/* Footer discret */}
      <p className="fixed bottom-4 text-sm text-gray-500">
        Connecté avec MQTT · ESP32 · Next.js
      </p>
    </main>
  );
}
