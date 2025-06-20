import React, { useEffect, useRef, useState } from 'react';

const CANVAS_WIDTH = 800;
const CANVAS_HEIGHT = 600;
const MAX_SPEED = 5;
const TURN_SPEED = 0.05;

export default function TurtleGame() {
    const canvasRef = useRef(null);
    const [turtle, setTurtle] = useState({
        x: CANVAS_WIDTH / 2,
        y: CANVAS_HEIGHT / 2,
        angle: 0,
        speed: 0,
    });
    const velocityRef = useRef(0);
    const wsRef = useRef(null);
    const kcalImpulseRef = useRef(0);

    // Handle WebSocket kcal updates
    useEffect(() => {
        wsRef.current = new WebSocket('ws://localhost:8080');
        wsRef.current.onmessage = (event) => {
            try {
                const data = JSON.parse(event.data);
                if (typeof data.work_kcal === 'number') {
                    // Translate kcal into forward impulse
                    const impulse = data.work_kcal * 5000; // Tune this multiplier
                    // console.log(`impulse=${impulse}`);
                    kcalImpulseRef.current += impulse;
                }
            } catch (e) {
                console.warn('Invalid data from WebSocket:', e);
            }
        };
        return () => wsRef.current?.close();
    }, [wsRef, kcalImpulseRef]);

    // Handle gamepad input
    function readGamepadInput() {
        const gp = navigator.getGamepads?.()[0];
        if (!gp) return 0;

        // Use left stick or D-pad left/right
        let turn = 0;
        // const axis = gp.axes?.[0] || 0;
        const leftPressed = gp.buttons?.[14]?.pressed;
        const rightPressed = gp.buttons?.[15]?.pressed;

        // if (Math.abs(axis) > 0.1) turn = axis;
        if (leftPressed) turn = -1;
        if (rightPressed) turn = 1;

        // if(turn !== 0) {
        //     console.log(turn);
        // }
        return turn;
    }

    // Game loop
    useEffect(() => {
        const draw = () => {
            const ctx = canvasRef.current.getContext('2d');

            velocityRef.current += kcalImpulseRef.current;
            // console.log(`velocityRef.current=${velocityRef.current}`);

            const turn = readGamepadInput();
            const angle = turtle.angle + turn * TURN_SPEED;

            const speed = Math.min(MAX_SPEED, velocityRef.current);
            const dx = Math.cos(angle) * speed;
            const dy = Math.sin(angle) * speed;

            const newX = (turtle.x + dx + CANVAS_WIDTH) % CANVAS_WIDTH;
            const newY = (turtle.y + dy + CANVAS_HEIGHT) % CANVAS_HEIGHT;

            const updatedTurtle = {
                x: newX,
                y: newY,
                angle,
                speed,
            };
            // console.log(updatedTurtle);

            setTurtle(updatedTurtle);

            // Clear and draw
            ctx.clearRect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT);

            // Draw turtle (circle + triangle for direction)
            ctx.save();
            ctx.translate(updatedTurtle.x, updatedTurtle.y);
            ctx.rotate(updatedTurtle.angle);

            // Turtle body
            ctx.fillStyle = 'green';
            ctx.beginPath();
            ctx.arc(0, 0, 20, 0, Math.PI * 2);
            ctx.fill();

            // Turtle head
            ctx.fillStyle = 'darkgreen';
            ctx.beginPath();
            ctx.moveTo(0, -20);
            ctx.lineTo(10, -10);
            ctx.lineTo(-10, -10);
            ctx.closePath();
            ctx.fill();

            ctx.restore();

            requestAnimationFrame(draw);
        };

        draw();
    }, [setTurtle, kcalImpulseRef, turtle]);

    return (
        <div>
            <h2>🚴 Turtle Pedal Game 🐢</h2>
            <canvas
                ref={canvasRef}
                width={CANVAS_WIDTH}
                height={CANVAS_HEIGHT}
                style={{ border: '1px solid #ccc' }}
            />
        </div>
    );
}
