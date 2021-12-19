package main

import (
	"context"
	//"fmt"
	"log"

	firebase "firebase.google.com/go/v4"
	"firebase.google.com/go/v4/db"
	"google.golang.org/api/option"
)

var firebase_URL = "https://qrsystem-1b711-default-rtdb.asia-southeast1.firebasedatabase.app"
var firebase_auth_path string = "./firebase_certs/qrsystem-1b711-firebase-adminsdk-z1k9l-087869e273.json"

func FirebaseUpdateEntry(ctx context.Context, client *db.Client, storename string, table string, seq string, 
	phone string, datatype string, data map[string]interface{} ){

		ref := client.NewRef("Order/"+storename+"/"+table+"/"+seq+"/"+phone+"/"+datatype)
		ref.Update(ctx,data)
}

func FirebaseUpdateSingleField(ctx context.Context, client *db.Client, storename string, table string, seq string, 
	phone string, datatype string, value_to_modify string, newdata string){

		ref := client.NewRef("Order/"+storename+"/"+table+"/"+seq+"/"+phone+"/"+datatype)
		
		new_map := map[string]interface{}{
			value_to_modify:newdata,
		}

		ref.Update(ctx,new_map)
	}

func get_firebase_context_certified()(context.Context, *db.Client){

	ctx := context.Background()
	conf := &firebase.Config{
		DatabaseURL: firebase_URL,
	}

	opt := option.WithCredentialsFile(firebase_auth_path);

	// Initialize the app with a service account, granting admin privileges
	app, err := firebase.NewApp(ctx, conf, opt)
	if err != nil {
		log.Fatalln("Error initializing app:", err)
	}

	client, err := app.Database(ctx)
	if err != nil {
		log.Fatalln("Error initializing database client:", err)
	}

	return ctx,client;
}


func example_test() {
	
	//ctx, client := get_firebase_context_certified()
/*
	// As an admin, the app has access to read and write all data, regradless of Security Rules
	ref := client.NewRef("users/a")
	var data map[string]interface{}
	if err := ref.Get(ctx, &data); err != nil {
		log.Fatalln("Error reading from database:", err)
	}
	fmt.Println(data)

	update_data := map[string]interface{}{
		"username":"d",
	}

	ref.Update(ctx,update_data)
*/
}