import { reactive } from "vue";
import { io } from "socket.io-client";

export const state = reactive({
    connected: false,
    fooEvents: [],
    barEvents: [],
})

const URL = "http://localhost:3000";
const socket = io(URL);

socket.on("connect", () => {
    state.connected = true;
    console.log("Connected to socket server");
})
socket.on("disconnect", () => {
    state.connected = false;
    console.log("Disconnected from socket server");
})


export default socket;


