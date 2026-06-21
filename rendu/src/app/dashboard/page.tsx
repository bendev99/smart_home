import LedController from "@/components/LedController";
import SensorDisplay from "@/components/SensorDisplay";

const Dashboard = () => {
  return (
    <div className="min-h-screen flex flex-col items-center justify-center text-white bg-teal-950">
      <div className="flex flex-col h-[50%] w-[50%] shadow shadow-teal-600 rounded-2xl p-5">
        <LedController />
        <SensorDisplay />
      </div>
    </div>
  );
};

export default Dashboard;
