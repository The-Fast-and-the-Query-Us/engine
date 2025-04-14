import * as ReactDOM from "react-dom/client";
import React from "react";
import Visibility from "./Visibility.tsx"

const root = ReactDOM.createRoot(document.getElementById("root")!);
root.render(
  <React.StrictMode>
    <Visibility />
  </React.StrictMode>,
);
