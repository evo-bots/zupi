package main

import (
	"github.com/evo-bots/zupi/bots/zpi1/brain"
)

type brainCmd struct {
	Name   string
	Accept []string
}

func (c *brainCmd) Execute(args []string) error {
	br, err := brain.NewBrain(c.Name, c.Accept)
	if err == nil {
		err = br.Run()
	}
	return err
}
