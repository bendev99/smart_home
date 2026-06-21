import LedController from "@/components/LedController";
import React from "react";

const Dashboard = () => {
  return (
    <div className="min-h-screen flex flex-col items-center justify-center text-white bg-teal-950">
      <LedController />
    </div>
  );
};

export default Dashboard;
