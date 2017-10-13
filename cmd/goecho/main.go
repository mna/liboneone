package main

import (
	"fmt"
	"log"
	"net"
)

func serve(c net.Conn) {
	defer c.Close()

	b := make([]byte, 100)
	for {
		n, err := c.Read(b)
		if err != nil {
			fmt.Printf("Read: %s\n", err)
			return
		}
		if _, err := c.Write(b[:n]); err != nil {
			fmt.Printf("Write: %s\n", err)
			return
		}
	}
}

func main() {
	ln, err := net.Listen("tcp", ":8080")
	if err != nil {
		log.Fatal(err)
	}
	for {
		conn, err := ln.Accept()
		if err != nil {
			log.Fatal(err)
		}
		go serve(conn)
	}
}
