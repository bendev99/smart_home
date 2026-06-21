import LedController from "@/components/LedController";
import SensorDisplay from "@/components/SensorDisplay";
import React from "react";

const Dashboard = () => {
  return (
    <div className="min-h-screen flex flex-col items-center justify-center text-white bg-teal-950">
      <LedController />
      <SensorDisplay />
    </div>
  );
};

export default Dashboard;
