import React, { useEffect, useState } from "react";
import { Chart } from "react-google-charts";

const ips = [
  "34.48.157.78",
  "34.162.126.210",
  "34.162.74.232",
  "34.56.166.106",
  "34.162.206.198",
  "34.135.72.123",
  "34.48.10.178",
];

const options = {
  title: "Crawler performance",
  curveType: "line",
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

type SpeedStats = {
  current: number;
  avg: number;
};

const Visilibity = () => {
  const [chartData, setChartData] = useState<DataPoint[]>();
  const [selectedIP, setSelectedIP] = useState("ALL");
  const [allStats, setAllStats] = useState<{
    overall: SpeedStats;
    perIP: Record<string, SpeedStats>;
  } | null>(null);

  useEffect(() => {
    const fetchLogs = (ip: string) =>
      fetch(`http://${ip}/logs`)
        .then((res) =>
          res.ok ? res.text() : Promise.reject(`Failed to fetch from ${ip}`),
        )
        .then((text) =>
          text
            .trim()
            .split("\n")
            .map((line) => parseInt(line))
            .filter((t) => !isNaN(t)),
        );

    const loadData = async () => {
      try {
        let allTimestamps: number[] = [];
        let perIPStats: Record<string, SpeedStats> = {};

        if (selectedIP === "ALL") {
          const results = await Promise.allSettled(ips.map(fetchLogs));

          for (let i = 0; i < results.length; i++) {
            const ip = ips[i];
            const result = results[i];

            if (result.status === "fulfilled") {
              const timestamps = result.value.sort((a, b) => a - b);
              if (timestamps.length >= 2) {
                const rawRates: number[] = [];
                for (let j = 1; j < timestamps.length; j++) {
                  const delta = timestamps[j] - timestamps[j - 1];
                  const rate = (4095 / delta) * 1000;
                  rawRates.push(rate);
                }
                const current = rawRates[rawRates.length - 1];
                const avg =
                  rawRates.reduce((sum, r) => sum + r, 0) / rawRates.length;
                perIPStats[ip] = { current, avg };
                allTimestamps.push(...timestamps);
              }
            } else {
              console.warn(result.reason);
            }
          }
        } else {
          const timestamps = await fetchLogs(selectedIP);
          allTimestamps = timestamps;
        }

        allTimestamps.sort((a, b) => a - b);
        if (allTimestamps.length < 2) {
          setChartData(undefined);
          setAllStats(null);
          return;
        }

        const rawRates: number[] = [];
        const times: Date[] = [];

        for (let i = 1; i < allTimestamps.length; i++) {
          const delta = allTimestamps[i] - allTimestamps[i - 1];
          const rate = (4095 / delta) * 1000;
          rawRates.push(rate);
          times.push(new Date(allTimestamps[i]));
        }

        const data: (string | Date | number)[][] = [
          ["Date", "Rate", "Rolling Avg"],
        ];

        for (let i = 0; i < rawRates.length; i++) {
          const windowStart = Math.max(0, i - 19);
          const window = rawRates.slice(windowStart, i + 1);
          const avg = window.reduce((sum, x) => sum + x, 0) / window.length;
          data.push([times[i], rawRates[i], avg]);
        }

        setChartData(data as DataPoint[]);

        if (selectedIP === "ALL") {
          const current = rawRates[rawRates.length - 1];
          const avg = rawRates.reduce((sum, r) => sum + r, 0) / rawRates.length;
          setAllStats({ overall: { current, avg }, perIP: perIPStats });
        } else {
          setAllStats(null);
        }
      } catch (error) {
        console.error("Data loading error:", error);
        setChartData(undefined);
        setAllStats(null);
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

          {selectedIP === "ALL" && allStats && (
            <div>
              <hr />
              <p>
                <strong>Overall current speed:</strong>{" "}
                {allStats.overall.current.toFixed(2)} docs/second
              </p>
              <p>
                <strong>Overall average speed:</strong>{" "}
                {(((chartData.length - 1) * 4096) /
                  (chartData[chartData.length - 1][0] - chartData[1][0])) *
                  1000}{" "}
                docs/second
              </p>
            </div>
          )}
        </>
      ) : (
        <p>Loading...</p>
      )}
    </>
  );
};

export default Visilibity;
