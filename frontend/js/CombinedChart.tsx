import { Chart } from "react-google-charts";

const options = {
  title: "Combined performance",
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
  vAxes: {
    0: { title: "Documents Written" },
    1: { title: "Crawl Speed (docs/sec)" },
  },
  series: {
    0: { targetAxisIndex: 0 },
    1: { targetAxisIndex: 1 },
  },
};

const CombinedChart = ({ data }: { data: Record<string, Date[]> }) => {
  const timestamps = Object.values(data)
    .flat()
    .sort((a, b) => a.getTime() - b.getTime());

  const cumulativeDocs: [Date, number][] = timestamps.map((date, index) => [
    date,
    (index + 1) * 4095,
  ]);

  const avgSpeeds: (number | null)[] = timestamps.map((currentTime, i) => {
    const windowStart = new Date(currentTime.getTime() - 5 * 60 * 1000);
    let count = 0;
    for (let j = i; j >= 0; j--) {
      const ts = timestamps[j];
      if (ts === undefined) continue;

      if (ts >= windowStart) {
        count++;
      } else {
        break;
      }
    }
    return (count / 300) * 4095;
  });

  const chartData: [Date, number, number | null][] = timestamps.map(
    (date, i) => {
      const cumulative = cumulativeDocs[i]?.[1] ?? 0;
      const avg = avgSpeeds[i] ?? null;
      return [date, cumulative, avg];
    },
  );

  const fullChartData = [
    ["Date", "Documents Written", "Crawl Speed (docs/sec)"],
    ...chartData,
  ];

  return (
    <Chart
      chartType="LineChart"
      width="100%"
      height="500px"
      data={fullChartData}
      options={options}
    />
  );
};

export default CombinedChart;
