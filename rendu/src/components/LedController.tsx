"use client";

import { useEffect, useState, useRef } from "react";
import mqtt from "mqtt";
import { BsToggle2Off, BsToggle2On } from "react-icons/bs";

const BROKER_URL = "ws://10.248.17.37:9001"; // IP du PC
const TOPIC_CMD = "esp32/led/cmd";
const TOPIC_STATE = "esp32/led/state";

export default function LedController() {
  const [ledOn, setLedOn] = useState(false);
  const clientRef = useRef<any>(null);

  useEffect(() => {
    // Connexion au broker MQTT
    const client = mqtt.connect(BROKER_URL, {
      clientId: "nextjs_" + Math.random().toString(16).substr(2, 8),
    });

    client.on("connect", () => {
      console.log("MQTT connecté (WebSocket)");
      client.subscribe(TOPIC_STATE, (err) => {
        if (!err) console.log("Abonnement à", TOPIC_STATE);
      });
    });

    client.on("message", (topic: string, message: Buffer) => {
      if (topic === TOPIC_STATE) {
        const state = message.toString();
        setLedOn(state === "on");
      }
    });

    clientRef.current = client;

    return () => {
      client.end();
    };
  }, []);

  const toggleLed = () => {
    if (clientRef.current) {
      const command = ledOn ? "led_off" : "led_on";
      clientRef.current.publish(TOPIC_CMD, command);
    }
  };

  return (
    <div className="flex flex-col items-center justify-center gap-3">
      <h2 className="text-2xl font-bold">Contrôle de la LED ESP32-S3</h2>
      <p className="text-xl font-semibold">
        État actuel : {ledOn ? "Allumée" : "Éteinte"}
      </p>

      {ledOn ? (
        <img src="/led_on.png" alt="LED ON" className="h-36 w-36" />
      ) : (
        <img src="/led_off.png" alt="LED OFF" className="h-36 w-36" />
      )}

      <button
        onClick={toggleLed}
        className={`flex w-[15%] gap-2 items-center justify-center text-md py-1 rounded-full shadow-lg shadow-teal-700/30 hover:shadow-teal-700/40 cursor-pointer transition-all ${ledOn && "bg-amber-500/10"}`}
      >
        {ledOn ? (
          <BsToggle2On size={38} color="yellow" />
        ) : (
          <BsToggle2Off size={38} />
        )}
      </button>
    </div>
  );
}
