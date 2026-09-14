using System;
using System.Text;
using ENet;

namespace ENetClient
{
    class Program
    {
        static void Main(string[] args)
        {
            // 1. Initialize the ENet library
            Library.Initialize();

            // 2. Create a client host
            using (Host client = new Host())
            {
                // 3. Create an address for the server we want to connect to
                Address address = new Address();
                address.SetHost("127.0.0.1"); // Localhost (your own PC)
                address.Port = 7777;          // The port your C++ server is listening on

                // 4. Create the client and connect to the server (2 channels)
                client.Create();
                Peer peer = client.Connect(address, 2);

                Console.WriteLine("Connecting to server...");

                // --- Timeout setup ---
                const double ConnectTimeoutSeconds = 8.0;
                DateTime connectStart = DateTime.UtcNow;
                bool connected = false;
                bool timedOut = false;

                // 5. Main loop to service the network and handle events
                while (true)
                {
                    // Service the network for 15ms. Returns int (>0 if event fired).
                    if (client.Service(15, out Event netEvent) > 0)
                    {
                        switch (netEvent.Type)
                        {
                            case EventType.Connect:
                                Console.WriteLine($"Successfully connected to server! Peer ID: {netEvent.Peer.ID}");
                                connected = true;

                                // Send a message to the server upon connecting
                                string message = "Hello from the C# client!";
                                Packet packet = new Packet();
                                packet.Create(Encoding.UTF8.GetBytes(message));
                                peer.Send(0, ref packet); // Pass by ref, channel 0
                                packet.Dispose();
                                Console.WriteLine($"Sent message: '{message}'");
                                break;

                            case EventType.Receive:
                                // We received a packet from the server
                                byte[] data = new byte[netEvent.Packet.Length];
                                netEvent.Packet.CopyTo(data);
                                string receivedMessage = Encoding.UTF8.GetString(data);
                                Console.WriteLine($"Received from server: '{receivedMessage}'");

                                // Clean up the packet
                                netEvent.Packet.Dispose();
                                break;

                            case EventType.Disconnect:
                                Console.WriteLine($"Disconnected from server. Peer ID: {netEvent.Peer.ID}");
                                connected = false;
                                break;

                            case EventType.Timeout:
                                Console.WriteLine($"Connection timed out. Peer ID: {netEvent.Peer.ID}");
                                connected = false;
                                break;
                        }
                    }

                    // --- Timeout check (only while not connected) ---
                    if (!connected)
                    {
                        double elapsed = (DateTime.UtcNow - connectStart).TotalSeconds;
                        if (elapsed >= ConnectTimeoutSeconds)
                        {
                            Console.WriteLine($"Connection timed out after {ConnectTimeoutSeconds} seconds.");
                            timedOut = true;
                            break;
                        }
                    }

                    // If we lose connection after having connected, break the loop
                    if (!connected && netEvent.Type != EventType.None)
                    {
                        break;
                    }

                    // Small delay to keep CPU usage low
                    System.Threading.Thread.Sleep(1);
                }

                // --- Clean up the pending peer if we timed out ---
                if (timedOut)
                {
                    peer.DisconnectNow(0);
                }

                // 6. Flush any pending outgoing packets before exiting
                client.Flush();
            }

            // 7. Deinitialize the library
            Library.Deinitialize();
            Console.WriteLine("Client shut down cleanly.");
            Console.ReadLine();
        }
    }
}