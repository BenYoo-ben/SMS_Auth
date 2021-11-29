package main

import (
	"context"
	"fmt"
	"log"

	firebase "firebase.google.com/go/v4"
	"google.golang.org/api/option"
)

type Data struct {
	TypeClient string `json:",omitempty"`
}

var responseData Data

func main() {
	ctx := context.Background()
	conf := &firebase.Config{
		DatabaseURL: "https://qrsystem-1b711-default-rtdb.asia-southeast1.firebasedatabase.app",
	}
	// Fetch the service account key JSON file contents
	opt := option.WithCredentialsFile("./qrsystem-1b711-firebase-adminsdk-z1k9l-087869e273.json")

	// Initialize the app with a service account, granting admin privileges
	app, err := firebase.NewApp(ctx, conf, opt)
	if err != nil {
		log.Fatalln("Error initializing app:", err)
	}

	client, err := app.Database(ctx)
	if err != nil {
		log.Fatalln("Error initializing database client:", err)
	}

	// As an admin, the app has access to read and write all data, regradless of Security Rules
	ref := client.NewRef("restricted_access/secret_document")
	var data map[string]interface{}
	if err := ref.Get(ctx, &data); err != nil {
		log.Fatalln("Error reading from database:", err)
	}
	fmt.Println(data)

}