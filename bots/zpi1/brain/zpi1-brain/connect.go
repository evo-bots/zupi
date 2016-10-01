package main

import (
	conn "github.com/evo-bots/zupi/bots/zpi1/brain/connector"
)

type connectCmd struct {
	Name string
}

func (c *connectCmd) Execute(args []string) error {
	connector, err := conn.NewConnector(c.Name, args)
	if err == nil {
		err = connector.Run()
	}
	return err
}
