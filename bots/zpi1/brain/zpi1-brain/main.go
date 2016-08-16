package main

import (
	"log"

	"github.com/evo-bots/zupi/bots/zpi1/brain"
)

func main() {
	br, err := brain.NewBrain()
	if err == nil {
		err = br.Run()
	}
	if err != nil {
		log.Fatalln(err.Error())
	}
}
