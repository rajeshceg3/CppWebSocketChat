import { useState, useEffect, useRef, useCallback } from 'react';

const useWebSocket = (url) => {
  const [messages, setMessages] = useState([]);
  const [readyState, setReadyState] = useState(WebSocket.CONNECTING);
  const [error, setError] = useState(null);
  const wsRef = useRef(null);

  useEffect(() => {
    if (!url) return;

    const ws = new WebSocket(url);
    wsRef.current = ws;

    setReadyState(ws.readyState); // Initial state

    ws.onopen = () => {
      console.log('WebSocket Connected');
      setReadyState(ws.readyState);
      setError(null);
    };

    ws.onmessage = (event) => {
      console.log('WebSocket Message Received:', event.data);
      try {
        const parsedMessage = JSON.parse(event.data);
        // Simple validation for our protocol: expect an object with 'type' and 'payload'
        if (typeof parsedMessage === 'object' && parsedMessage !== null && 'type' in parsedMessage && 'payload' in parsedMessage) {
          setMessages((prevMessages) => [...prevMessages, parsedMessage]);
        } else {
          console.warn('Received message does not match expected protocol structure:', parsedMessage);
          // Optionally, add non-protocol messages to a different state or handle them
        }
      } catch (e) {
        console.error('Failed to parse JSON message:', e);
        // Handle non-JSON messages or parsing errors if necessary
        // For now, we add raw data if it's not JSON, prefixed for clarity
        setMessages((prevMessages) => [...prevMessages, {type: 'raw_data', payload: {text: event.data, timestamp: new Date().toISOString(), user_id: 'System'}}]);
      }
    };

    ws.onerror = (event) => {
      console.error('WebSocket Error:', event);
      // For security reasons, the specific error event doesn't bubble up detailed info to JS
      // We know an error occurred, but not much more.
      setError(new Error('WebSocket error occurred. Check console for details.'));
      setReadyState(ws.readyState);
    };

    ws.onclose = (event) => {
      console.log('WebSocket Disconnected. Code:', event.code, 'Reason:', event.reason);
      setReadyState(ws.readyState);
      // Optionally, set an error if the disconnect was unexpected
      if (!event.wasClean) {
        setError(new Error(`WebSocket closed uncleanly. Code: ${event.code}, Reason: ${event.reason}`));
      }
    };

    return () => {
      if (ws.readyState === WebSocket.OPEN || ws.readyState === WebSocket.CONNECTING) {
        ws.close();
        console.log('WebSocket connection closed on cleanup.');
      }
      wsRef.current = null;
      setReadyState(WebSocket.CLOSED); // Ensure final state is CLOSED
    };
  }, [url]); // Re-run effect if URL changes

  const sendMessage = useCallback((messageObject) => {
    if (wsRef.current && wsRef.current.readyState === WebSocket.OPEN) {
      try {
        const jsonMessage = JSON.stringify(messageObject);
        wsRef.current.send(jsonMessage);
        console.log('WebSocket Message Sent:', jsonMessage);
      } catch (e) {
        console.error('Failed to stringify message object:', e);
      }
    } else {
      console.warn('WebSocket not open. Message not sent:', messageObject);
      setError(new Error('WebSocket is not connected. Cannot send message.'));
    }
  }, []); // No dependencies, wsRef.current is managed by useEffect

  return { messages, readyState, error, sendMessage };
};

export default useWebSocket;
