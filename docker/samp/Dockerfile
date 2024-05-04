FROM debian:buster-slim

WORKDIR /samp

COPY server_files ./

RUN dpkg --add-architecture i386

RUN apt-get update && apt-get install -y lib32stdc++6 wget psmisc

RUN chmod 777 *

EXPOSE 7777/udp
EXPOSE 7777/tcp

CMD ["./samp03svr"]