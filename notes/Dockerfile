FROM ubuntu:latest

RUN /usr/sbin/useradd --no-create-home -u 1000 ctf

WORKDIR /home/ctf

COPY notes .
COPY flag.txt .

RUN apt-get update
RUN apt-get install -y socat

RUN chown -R root:root /home/ctf

USER ctf

EXPOSE 1337

CMD socat TCP-LISTEN:1337,reuseaddr,fork EXEC:"/home/ctf/notes"

