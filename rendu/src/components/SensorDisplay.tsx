"use client";

import { useEffect, useState } from "react";
import mqtt from "mqtt";
import { WiHumidity } from "react-icons/wi";
import { FaTemperatureHalf } from "react-icons/fa6";
import { BiRuler } from "react-icons/bi";

const BROKER_URL = "ws://10.248.17.37:9001";
const TOPIC_DHT = "esp32/dht";
const TOPIC_DISTANCE = "esp32/distance";

export default function SensorDisplay() {
  const [temp, setTemp] = useState("--");
  const [hum, setHum] = useState("--");
  const [distance, setDistance] = useState("--");

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
      <div className="bg-teal-800 rounded-xl text-center p-6 shadow-lg">
        <p className="flex items-center text-md text-center justify-center gap-3 text-white">
          Température <FaTemperatureHalf className="text-white" />
        </p>
        <p className="text-3xl font-bold text-white">{temp} °C</p>
      </div>

      <div className="bg-teal-800 rounded-xl text-center p-6 shadow-lg">
        <p className="flex  items-center text-md text-center justify-center gap-3 text-white">
          Humidité <WiHumidity className="text-white" />
        </p>
        <p className="text-3xl font-bold text-white">{hum} %</p>
      </div>

      <div className="bg-teal-800 rounded-xl text-center p-6 shadow-lg">
        <p className="flex items-center text-md text-center justify-center gap-3 text-white">
          Distance <BiRuler className="text-white" />
        </p>
        <p className="text-3xl font-bold text-white">{distance} cm</p>
      </div>
    </div>
  );
}
