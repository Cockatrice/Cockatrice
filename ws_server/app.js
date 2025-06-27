import express from "express";
import { createServer } from "node:http";
import { Server } from "socket.io";

const app = express();
const server = createServer(app);
const io = new Server(server, {
  cors: {
    origin: "http://localhost:9000",
  },
});

app.get("/", (req, res) => {
  res.send("<h1>Hello world</h1>");
});

io.on("connection", (socket) => {
  console.log("a user connected");
  socket.on("disconnect", () => {
    console.log("user disconnected");
  });
  socket.on("join", (username, room) => {
    socket.join(room);
    // socket.to(room).emit("updateCards")
    console.log(`${username} joined room: ${room}`);
  });
  socket.on("sync-cards", ({ room, username, cards }) => {
    console.log(`syncing cards for room: ${room}`);
    console.log("the cards: ", username, cards);
    io.to(room).emit("updateCards", {
      player: {
        username: username,
        cards: cards,
      },
    });
    console.log(`sent updated cards for room: ${room}`);
  });
});

server.listen(3000, () => {
  console.log("server running at http://localhost:3000");
});
