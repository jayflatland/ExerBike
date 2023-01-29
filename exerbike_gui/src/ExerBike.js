import React, { useState, useCallback, useEffect } from 'react';
import useWebSocket, { ReadyState } from 'react-use-websocket';
import C3Chart from 'react-c3js';
import 'c3/c3.css';

export default function ExerBike() {
    const [socketUrl, setSocketUrl] = useState('ws://10.1.10.178/exerbike');
    const [totalKCalHistory, setTotalKCalHistory] = useState([]);
    const [totalKCal, setTotalKCal] = useState(0.0);

    const { lastMessage, readyState } = useWebSocket(socketUrl);

    useEffect(() => {
        if (lastMessage !== null) {
            var d = JSON.parse(lastMessage.data);
            console.log(d);
            var newTotalKCal = totalKCal + d.work_kcal;
            setTotalKCal(newTotalKCal);
            setTotalKCalHistory((prev) => prev.concat(newTotalKCal));
            // var samples = ['data'].concat(d.samples);
            // setData({columns: [samples]});
        }
    }, [lastMessage]);

    const connectionStatus = {
        [ReadyState.CONNECTING]: 'Connecting',
        [ReadyState.OPEN]: 'Open',
        [ReadyState.CLOSING]: 'Closing',
        [ReadyState.CLOSED]: 'Closed',
        [ReadyState.UNINSTANTIATED]: 'Uninstantiated',
    }[readyState];

    return (
        <div>
            <span>The WebSocket is currently {connectionStatus}</span>
            <h1>You Have Burned {totalKCal.toFixed(2)} Calories!</h1>
            <C3Chart data={{columns: [['data', ...totalKCalHistory]]}} />
        </div>
    );
};
