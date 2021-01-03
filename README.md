# MAD Films - Video Streaming Application

In recent times, due to the pandemic, the use of online video streaming applications like YouTube, Netflix, and Amazon Prime has increased exponentially, with a lot of new movies being released on many such applications almost everyday. So we plan to implement a **GUI based IP multicast video streaming application using C** which can allow a user at the client side to select which video he/she wants to watch and then stream that particular video using the **Any Source Multicast (ASM)** model. The connection between the server and individual clients will take place via **TCP protocol**, and the data (video) will be transmitted from the server to the clients via **UDP protocol**.
Using this project we aim to learn about what is the nature of multimedia traffic, how it can be controlled, and main protocol based implementation of transmitting video data over the internet. The project will also help us to understand GUI development in C language.

## Tools and Technologies:

- This project is implemented using C language:
   - Socket Programming
   - Multithreading
   - GTK Package
   - VLC Media Player Package
- Protocols Used:
   - TCP: For fetching the channel list
   - UDP: For streaming the video
   
## Installation of Dependencies:

To run this project smoothly, we need to install dependencies for **VLC Media Player**, **GTK Package** and **ffmpeg package**. To install them, run the following commands before running the project:
- **ffmpeg:** `sudo apt install ffmpeg` (Used for converting the videos into a streamable format)
- **libvlc:** `sudo apt install libvlc-dev` (C API for supporting VLC Media Player)
- **VLC Media Player:** `sudo apt install vlc` *Note: If VLC player is alreay installed in your system using `snap` command then kindly uninstall that and reinstall it using `apt` command.*
- GTK Packages for GUI development:
   - **GTK2:** `sudo apt install libgtk2.0-dev`
   - **GTK3:** `sudo apt install libgtk-3-dev`
   
## How to Run the Project:

1. Before running the project, you need to check which interfaces on your machine support Multicasting facility and which of them are running at the moment. For that, use `ifconfig` command. Accordingly, edit the `IF_NAME` used in the `client.c` file.
2. Now you need to convert the videos you wish to stream into streamable formats. For that, type the command:
   - `ffmpeg -i INPUT_FILENAME.mp4 -f mpegts OUTPUT_FILENAME.mp4`
   - *Note: Repeat the command for all the videos that you wish to stream by changing the input and output file names. Here we have used .mp4 extension, but this project works well with all other types of video extensions.*
3. **Compiling the Project:**
   - **server.c** (../MAD Films/Server): `gcc server.c -o server -lpthread`
   - **client.c** (../MAD Films/Client): `gcc -o client client.c `pkg-config --libs gtk+-2.0 libvlc` `pkg-config --cflags gtk+-2.0 libvlc``
4. **Run the Project:**
   - **server.c** (../MAD Films/Server): `./server`
   - **client.c** (../MAD Films/Client): `sudo ./client` *Note: Writing `sudo` is necessary to run the client file in socket programming*

## References:

- For understanding file transfer using UDP:
   - https://www.geeksforgeeks.org/c-program-for-file-transfer-using-udp/
- For understanding IP Multicasting:
   - https://www.tenouk.com/Module41c.html
- For GUI development using GTK package:
   - https://developer.gnome.org/gtk3/stable/
   - https://developer.gnome.org/gtk2/stable/
- For playing the video using VLC Media Player:
   - https://www.videolan.org/developers/vlc/doc/doxygen/html/libvlc-module_8c.html
   - https://progur.com/2017/06/how-to-use-vlc-api-in-c.html
   - https://stackoverflow.com/questions/10116783/a-simple-c-program-to-play-mp3-using-libvlc
   
   ## Contributors

---

| [Dipika Pawar](https://github.com/DipikaPawar12)                                                                                                            | [Aanshi Patwari](https://github.com/aanshi18)                                                                                                            | [Miracle Rindani](https://github.com/mrindani)                                                                                                |

