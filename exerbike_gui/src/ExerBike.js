import React, { useState, useRef, useEffect } from 'react';
import useWebSocket, { ReadyState } from 'react-use-websocket';
import C3Chart from 'react-c3js';
import 'c3/c3.css';

export default function ExerBike(props) {
    const [socketUrl] = useState('ws://10.1.10.177/exerbike');
    const [state, setState] = useState({
        "times": [],
        "kcals": [],
        "totalKCals": [],
        "totalKCal": 0.0,
        "startTime": undefined
    });

    const { lastMessage, readyState } = useWebSocket(socketUrl);

    const canvasRef = useRef(null)
    // const canvas = canvasRef.current
    // const context = canvas.getContext('2d')

    useEffect(() => {
        if (lastMessage !== null) {
            var d = JSON.parse(lastMessage.data);
            console.log(d);

            var now = Date.now();
            setState((prevState) => { return {
                "times": prevState.times.concat(now),
                "kcals": prevState.kcals.concat(d.work_kcal),
                "totalKCals": prevState.kcals.concat(prevState.totalKCal + d.work_kcal),
                "totalKCal": prevState.totalKCal + d.work_kcal,
                "startTime": prevState.startTime === undefined ? now : prevState.startTime
            }; });
        }

        const canvas = canvasRef.current
        const context = canvas.getContext('2d')
        //Our first draw
        context.fillStyle = '#0000ff';
        context.fillRect(0, 0, context.canvas.width, context.canvas.height);
  
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
            <h1>You Have Burned {state.totalKCal.toFixed(2)} Calories!</h1>
            <C3Chart data={{
                x: 'x',
                columns: [
                    ['x', ...state.times],
                    ['kcal', ...state.kcals]
                ],
            }} />
            <canvas ref={canvasRef} {...props}/>
        </div>
    );
};
