import React, { useEffect, useState } from "react";
import { Chart } from "react-google-charts";

const ips = ["34.162.206.198"];

const options = {
  title: "Crawler performance",
  curveType: "function",
  legend: { position: "bottom" },
  hAxis: {
    title: "Date",
    format: "MMM dd, HH:mm",
    gridlines: { count: 3 },
    viewWindow: {
      max: new Date(),
    },
  },
};

type DataPoint = [Date, number, number];

const Visilibity = () => {
  const [chartData, setChartData] = useState<DataPoint[]>();
  const [selectedIP, setSelectedIP] = useState("ALL");

  useEffect(() => {
    const fetchLogs = (ip: string) =>
      fetch(`http://${ip}:8082/logs`)
        .then((res) => res.ok ? res.text() : Promise.reject(`Failed to fetch from ${ip}`))
        .then((text) =>
          text.trim().split("\n").map((line) => parseInt(line)).filter((t) => !isNaN(t))
        );

    const loadData = async () => {
      try {
        let allTimestamps: number[] = [];

        if (selectedIP === "ALL") {
          const results = await Promise.allSettled(ips.map(fetchLogs));
          results.forEach((res) => {
            if (res.status === "fulfilled") {
              allTimestamps.push(...res.value);
            } else {
              console.warn(res.reason);
            }
          });
        } else {
          const timestamps = await fetchLogs(selectedIP);
          allTimestamps = timestamps;
        }

        allTimestamps.sort((a, b) => a - b);

        if (allTimestamps.length < 2) return;

        const rawRates: number[] = [];
        const times: Date[] = [];

        for (let i = 1; i < allTimestamps.length; i++) {
          const delta = allTimestamps[i] - allTimestamps[i - 1];
          const rate = (4095 / delta) * 1000;
          rawRates.push(rate);
          times.push(new Date(allTimestamps[i]));
        }

        const data: (string | Date | number)[][] = [["Date", "Rate", "Rolling Avg"]];

        for (let i = 0; i < rawRates.length; i++) {
          const windowStart = Math.max(0, i - 19);
          const window = rawRates.slice(windowStart, i + 1);
          const avg = window.reduce((sum, x) => sum + x, 0) / window.length;
          data.push([times[i], rawRates[i], avg]);
        }

        setChartData(data as DataPoint[]);
      } catch (error) {
        console.error("Data loading error:", error);
      }
    };

    loadData();
  }, [selectedIP]);

  return (
    <>
      <label htmlFor="ip-select">Select server: </label>
      <select
        id="ip-select"
        value={selectedIP}
        onChange={(e) => setSelectedIP(e.target.value)}
      >
        <option value="ALL">All servers</option>
        {ips.map((ip) => (
          <option key={ip} value={ip}>
            {ip}
          </option>
        ))}
      </select>

      {chartData !== undefined ? (
        <>
          <Chart
            chartType="LineChart"
            width="100%"
            height="400px"
            data={chartData}
            options={options}
          />
          <p>
            Last speed: {chartData.length > 1
              ? chartData[chartData.length - 1][1].toFixed(2)
              : "N/A"}
            {" "}docs/second
          </p>
        </>
      ) : (
        <p>Loading...</p>
      )}
    </>
  );
};

export default Visilibity;
