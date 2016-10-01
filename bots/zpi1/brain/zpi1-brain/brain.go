package main

import (
	"github.com/evo-bots/zupi/bots/zpi1/brain"
)

type brainCmd struct {
}

func (c *brainCmd) Execute(args []string) error {
	br, err := brain.NewBrain()
	if err == nil {
		err = br.Run()
	}
	return err
}
