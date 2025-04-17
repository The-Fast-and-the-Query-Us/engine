import { useEffect, useState } from "react";
import CombinedChart from "./CombinedChart";
import ServerChart from "./ServerChart";

const ips = [
  "34.172.33.202",
  "34.133.156.150",
  "35.226.211.111",
  "34.60.247.227",
  "34.28.188.170",
  "34.133.15.217",
  "34.44.160.138",
  "34.41.189.144",
  "35.232.15.103",
  "35.190.134.82",
  "35.192.205.143",
  "34.60.42.75",
  "35.223.132.175",
  "34.132.146.200",
  "34.29.21.161",
  "34.29.194.122",
  "35.239.244.95",
  "34.171.11.179",
];

const Visilibity = () => {
  const [data, setData] = useState<Record<string, Date[]>>({});

  useEffect(() => {
    const promises = ips.map(async (ip) => {
      return fetch(`http://${ip}/logs`)
        .then((response) => response.text())
        .then((text) => {
          const timestamps = text
            .trim()
            .split("\n")
            .map(Number)
            .filter((n) => !isNaN(n))
            .map((timestamp: number) => new Date(timestamp));

          return [ip, timestamps] as [string, Date[]];
        })
        .catch((error) => {
          console.error(`Failed to fetch from ${ip}:`, error);
          return [ip, []] as [string, Date[]];
        });
    });

    Promise.all(promises).then((entries) => {
      const result: Record<string, Date[]> = Object.fromEntries(entries);
      setData(result);
    });
  }, []);

  return (
    <>
      <CombinedChart data={data} />
      {ips.map((ip) => (
        <ServerChart data={data} ip={ip} key={ip} />
      ))}
    </>
  );
};

export default Visilibity;
