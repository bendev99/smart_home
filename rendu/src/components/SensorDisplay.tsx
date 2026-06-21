"use client";

import { useEffect, useState } from "react";
import mqtt from "mqtt";

const BROKER_URL = "ws://10.248.17.37:9001";
const TOPIC_DHT = "esp32/dht";
const TOPIC_DISTANCE = "esp32/distance";

export default function SensorDisplay() {
  const [temp, setTemp] = useState<string>("--");
  const [hum, setHum] = useState<string>("--");
  const [distance, setDistance] = useState<string>("--");

  useEffect(() => {
    const client = mqtt.connect(BROKER_URL, {
      clientId: "nextjs_sensors_" + Math.random().toString(16).substr(2, 8),
    });

    client.on("connect", () => {
      client.subscribe([TOPIC_DHT, TOPIC_DISTANCE]);
    });

    client.on("message", (topic: string, message: Buffer) => {
      const msg = message.toString();
      if (topic === TOPIC_DHT) {
        try {
          const data = JSON.parse(msg);
          setTemp(data.temp);
          setHum(data.hum);
        } catch (e) {
          console.error("Erreur parsing DHT:", e);
        }
      } else if (topic === TOPIC_DISTANCE) {
        setDistance(msg);
      }
    });

    return () => {
      client.end();
    };
  }, []);

  return (
    <div className="grid grid-cols-1 md:grid-cols-3 gap-4 my-8">
      <div className="bg-gray-800 rounded-xl p-6 shadow-lg">
        <p className="text-gray-400 text-sm">Température</p>
        <p className="text-3xl font-bold text-white">{temp} °C</p>
      </div>
      <div className="bg-gray-800 rounded-xl p-6 shadow-lg">
        <p className="text-gray-400 text-sm">Humidité</p>
        <p className="text-3xl font-bold text-white">{hum} %</p>
      </div>
      <div className="bg-gray-800 rounded-xl p-6 shadow-lg">
        <p className="text-gray-400 text-sm">Distance</p>
        <p className="text-3xl font-bold text-white">{distance} cm</p>
      </div>
    </div>
  );
}
