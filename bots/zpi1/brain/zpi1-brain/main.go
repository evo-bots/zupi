package main

import (
	"log"

	"github.com/evo-bots/zupi/bots/zpi1/brain"
)

// Version is the release version
const Version = "0.0.1"

// VersionSuffix is the suffix of version
var VersionSuffix = "-dev"

func main() {
	br, err := brain.NewBrain()
	if err == nil {
		err = br.Run()
	}
	if err != nil {
		log.Fatalln(err.Error())
	}
}
