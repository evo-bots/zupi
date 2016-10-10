package main

import (
	conn "github.com/evo-bots/zupi/bots/zpi1/brain/connector"
)

type connectCmd struct {
	Name    string
	Port    int
	WebRoot string `n:"www-root"`
}

func (c *connectCmd) Execute(args []string) error {
	connector, err := conn.NewConnector(c.Name, c.Port, c.WebRoot, args)
	if err == nil {
		err = connector.Run()
	}
	return err
}
