import React, { useEffect, useRef, useState } from 'react';

const GRAVITY = 0.4;
const FLAP_MULTIPLIER = 300;
const GROUND_Y = 400;
const BIRD_RADIUS = 20;

export default function FlappyBird() {
    const [position, setPosition] = useState(200);
    const [velocity, setVelocity] = useState(0);
    const canvasRef = useRef(null);
    const animationFrameRef = useRef();
    const wsRef = useRef();

    // WebSocket setup
    useEffect(() => {
        wsRef.current = new WebSocket('ws://localhost:8080');
        wsRef.current.onmessage = (event) => {
            try {
                const data = JSON.parse(event.data);
                if (typeof data.work_kcal === 'number') {
                    // Flap: apply an upward impulse
                    // console.log("data.work_kcal=", data.work_kcal);
                    setVelocity((v) => v - data.work_kcal * FLAP_MULTIPLIER);
                }
            } catch (e) {
                console.warn('Invalid JSON or missing work_kcal', e);
            }
        };

        return () => wsRef.current && wsRef.current.close();
    }, []);

    // Game loop
    useEffect(() => {
        const draw = () => {
            setVelocity((v) => v + GRAVITY);
            setPosition((pos) => {
                const newPos = pos + velocity;
                return Math.min(newPos, GROUND_Y - BIRD_RADIUS);
            });

            console.log(`velocity=${velocity}, position=${position}`);

            const canvas = canvasRef.current;
            const ctx = canvas.getContext('2d');
            ctx.clearRect(0, 0, canvas.width, canvas.height);

            // Draw bird
            ctx.beginPath();
            ctx.arc(100, position, BIRD_RADIUS, 0, 2 * Math.PI);
            ctx.fillStyle = 'gold';
            ctx.fill();

            // Ground line
            ctx.fillStyle = '#333';
            ctx.fillRect(0, GROUND_Y, canvas.width, 5);

            animationFrameRef.current = requestAnimationFrame(draw);
        };

        animationFrameRef.current = requestAnimationFrame(draw);

        return () => cancelAnimationFrame(animationFrameRef.current);
    }, [velocity]);

    return (
        <div>
            <h2>Flappy Calories</h2>
            <canvas ref={canvasRef} width={600} height={450} style={{ border: '1px solid #ccc' }} />
        </div>
    );
}
