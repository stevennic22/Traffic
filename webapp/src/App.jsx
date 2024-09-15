import React, { useState, useEffect } from "react";
import { createRoot } from "react-dom/client";

const sleep = (delay) => new Promise((resolve) => setTimeout(resolve, delay));
const tlBlank = "#ffffff";
const buttonOptions = [
  "Off",
  "RLGL",
  "Traffic",
  "Random",
  "Flash R",
  "Flash Y",
  "Flash G",
  "Flash A",
  "Shuffle",
];

const App = () => {

  const [device, setDevice] = useState(null);
  const [server, setServer] = useState(null);
  const [service, setService] = useState(null);

  const [pOverride, overridePid] = useState(false);
  const [aLOverride, overrideAllLights] = useState(false);

  const [pid, setPid] = useState(null);
  const [selectedOption, setOption] = useState(0);

  const [charSet, updateChars] = useState({
    service: "5d039600-00fc-3981-9985-653653ce831f",
    "5d039600-01fc-3981-9985-653653ce831f": {
      name: "pid",
      func: setPid,
    },
    "5d039600-02fc-3981-9985-653653ce831f": {
      name: "rl",
      defaultFill: "#ff0000",
    },
    "5d039600-03fc-3981-9985-653653ce831f": {
      name: "yl",
      defaultFill: "#ffff00",
    },
    "5d039600-04fc-3981-9985-653653ce831f": {
      name: "gl",
      defaultFill: "#00ff00",
    },
  });

  const [lights, setLights] = useState({
    rl: { color: uuidObjByName("rl").defaultFill, key: "red" },
    yl: { color: uuidObjByName("yl").defaultFill, key: "yel" },
    gl: { color: uuidObjByName("gl").defaultFill, key: "gre" },
  });

  function uuidObjByName(name) {
    for (const obj in charSet) {
      if (charSet[obj].name === name) {
        return charSet[obj];
      }
    }
    return null;
  }

  function addCharToCharSet(char) {
    const newCharSet = charSet[char.uuid];
    newCharSet["char"] = char;
    updateChars((charSet) => ({ ...charSet, ...newCharSet }));
  }

  function updateLight(enabled, light, newColor) {
    const interim = {};
    interim[light] = { color: newColor };

    if (!enabled) {
      interim[light].color = tlBlank;
    }

    setLights((lights) => ({ ...lights, ...interim }));
  }

  function charUpdated(event) {
    const uuidObj = charSet[event.target.uuid];
    const value = event.target.value.getInt8();

    if (uuidObj.name == "pid") {
      setPid(value);
    } else {
      updateLight(!value, uuidObj.name, uuidObj.defaultFill);
    }
  }

  async function writeToChar(clickedChar) {
    const uuidObj = uuidObjByName(clickedChar);
    let value = null;
    let setValue = 1;

    // 1 turns on light
    // 2 overrides other lights
    // 3 overrides process
    // 4 overrides process _and_ other lights

    if (aLOverride) {
      setValue += 1;
    }
    if (pOverride) {
      setValue += 2;
    }

    if (lights[clickedChar].color == "#ffffff") {
      value = Uint8Array.of(setValue);
    } else {
      value = Uint8Array.of(0);
    }

    try {
      if (uuidObj.char != null && server != null) {
        uuidObj.char.writeValueWithoutResponse(value);
      }
    } catch (error) {
      console.log(error);
    }
  }

  async function subscribeChar(char, uuidObj, cValue) {
    uuidObj = charSet[char.uuid];

    await uuidObj.char.addEventListener(
      "characteristicvaluechanged",
      charUpdated
    );
    await uuidObj.char.startNotifications();
  }

  const connectToDevice = async () => {
    const device = await navigator.bluetooth.requestDevice({
      filters: [
        {
          //name: "traffic32",
          services: [charSet.service],
        },
      ],
      //acceptAllDevices: true,
    });

    try {
      device.addEventListener("gattserverdisconnected", onDisconnected);
      setDevice(device);

      const server = await device.gatt.connect();
      setServer(server);

      const service = await server.getPrimaryService(charSet.service);
      setService(service);

      let pSvcChars = await service.getCharacteristics();

      for (let i = 0; i < pSvcChars.length; i++) {
        const cUuid = pSvcChars[i].uuid;
        let char = await service.getCharacteristic(cUuid);
        const currentChar = charSet[char.uuid];
        let dataViewValue = await char.readValue();
        let value = dataViewValue.getUint8();

        addCharToCharSet(char);
        subscribeChar(char, currentChar, value);

        if (currentChar.name === "pid") {
          currentChar.func(value);
        } else {
          if (value == 0) {
            updateLight(true, currentChar.name, currentChar.defaultFill);
          } else {
            updateLight(false, currentChar.name);
          }
        }

        await sleep(200);
      }
    } catch (err) {
      alert(err);
    }
  };

  async function disconnectBT() {
    server.disconnect();
  }

  const onDisconnected = () => {
    setDevice(null);
    setServer(null);
    setService(null);
    setPid(null);

    const rl = uuidObjByName("rl");
    rl.char.removeEventListener("characteristicvaluechanged", charUpdated);
    updateLight(true, "rl", rl.defaultFill);

    const yl = uuidObjByName("yl");
    yl.char.removeEventListener("characteristicvaluechanged", charUpdated);
    updateLight(true, "yl", yl.defaultFill);

    const gl = uuidObjByName("gl");
    gl.char.removeEventListener("characteristicvaluechanged", charUpdated);
    updateLight(true, "gl", gl.defaultFill);
  };

  const handleConnection = () => {
    if (!server || !server.connected) {
      // Call connect here
      connectToDevice();
    } else {
      // Call disconnect here
      disconnectBT();
    }
  };

  useEffect(() => {
    const handleUnload = () => {
      if (server != null) {
        disconnectBT();
      }
    };
    window.addEventListener("unload", handleUnload);
    return () => {
      window.removeEventListener("unload", handleUnload);
    };
  }, []);

  function Toggles() {
    return (
      <div>
        <label className="toggle" id="aloLabel">
          <input
            className="toggle-checkbox"
            type="checkbox"
            checked={aLOverride}
            onChange={() => {
              overrideAllLights(!aLOverride);
            }}
            id="allLightOverride"
          />
          <span className="toggle-switch"></span>
          <label className="toggle-label" htmlFor="allLightOverride">
            Override all lights
          </label>
        </label>

        <br />
        <br />

        <label className="toggle" id="poLabel">
          <input
            className="toggle-checkbox"
            checked={pOverride}
            onChange={() => {
              overridePid(!pOverride);
            }}
            type="checkbox"
            id="PidOverride"
          />
          <span className="toggle-switch"></span>
          <label className="toggle-label" htmlFor="PidOverride">
            Override Process
          </label>
        </label>
      </div>
    );
  }

  function ConnectionControls() {
    return (
      <div>
        <button
          id="connectionControl"
          onClick={() => {
            handleConnection();
          }}
        >
          {!server ? "Connect" : "Disconnect"}
        </button>
      </div>
    );
  }

  function CreateProcessSelectOptions() {
    let buttons = buttonOptions.map((o, idx) => {
      return (
        <option value={idx} key={o}>
          {o}
        </option>
      );
    });

    return (
      <div>
        <select
          id="newPID"
          value={selectedOption}
          onChange={(v) => setOption(v.target.value)}
        >
          {buttons}
        </select>
        <button
          id="setNewPID"
          onClick={() => {
            const uuidObj = uuidObjByName("pid");
            try {
              uuidObj.char.writeValueWithoutResponse(
                Uint8Array.of(selectedOption)
              );
            } catch (error) {
              console.log(error);
            }
          }}
        >
          Enable
        </button>
      </div>
    );
  }

  function ControlButtons() {
    let visiblePID = null;
    if (pid == null && server != null) {
      visiblePID = "Pending";
    } else if (pid != null) {
      visiblePID = pid;
    } else {
      visiblePID = "Disconnected"
    }
    return (
      <div className="bottom-buttons">
        <div className="status">
          <span>PID: </span>
          <span id="pid">{visiblePID}</span>
        </div>

        <br />
        <ConnectionControls />

        <br />
        <CreateProcessSelectOptions />

        <br />
        <br />
        <Toggles />
      </div>
    );
  }

  function CreateLight(props) {
    let autobot = "";
    if (props.light == "rl") {
      autobot = "matrix(0.67672326,0,0,0.67672326,-104.49217,177.67295)";
    } else if (props.light == "yl") {
      autobot = "matrix(0.67633656,0,0,0.67633656,-104.40389,408.87678)";
    } else {
      autobot = "matrix(0.67672326,0,0,0.67672326,-104.49217,639.67295)";
    }
    return (
      <path
        id={props.light}
        key={lights[props.light].key}
        fill={lights[props.light].color}
        onClick={() => {
          writeToChar(props.light);
        }}
        transform={autobot}
        d="m 486.89352,397.78333 c 0,71.41015 -57.88937,129.29953 -129.29953,129.29953 -71.41015,0 -129.29953,-57.88938 -129.29953,-129.29953 0,-71.41016 57.88938,-129.29953 129.29953,-129.29953 71.41016,0 129.29953,57.88937 129.29953,129.29953 z"
      />
    );
  }

  function TrafficLight() {
    return (
      <svg
        xmlnsdc="http://purl.org/dc/elements/1.1/"
        xmlnscc="http://creativecommons.org/ns#"
        xmlnsrdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
        xmlnssvg="http://www.w3.org/2000/svg"
        xmlns="http://www.w3.org/2000/svg"
        width="275"
        height="750"
        id="svg2"
        version="1.1"
      >
        <g id="layer1" transform="translate(0,-302.36218)">
          <g id="g3081">
            <rect
              rx="29.999998"
              ry="30"
              y="302.36218"
              x="0"
              height="100%"
              width="275"
              id="rect3052"
              fill="#000000"
            />

            <CreateLight light="rl" />
            <CreateLight light="yl" />
            <CreateLight light="gl" />
          </g>
        </g>
      </svg>
    );
  }

  return (
    <div className="container">
      <div className="left-column">
        <ControlButtons />
      </div>
      <div className="right-column">
        <TrafficLight />
      </div>
    </div>
  );
};

createRoot(document.getElementById("app")).render(
  <React.StrictMode>
    {/* <RouterProvider router={router} /> */}
    <App />
  </React.StrictMode>
);
