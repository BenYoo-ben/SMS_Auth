package main

import (
	"fmt"
	"log"
	"net/http"
	"html/template"
	"io/ioutil"
	"encoding/json"
)

type Page struct {
	Title string
	Body []byte
}

type Temp struct{
	Item1 string `json:"item1"`
	Item2 string `json:"item2"`
	Item3 string `json:"item3"`
}

func (p *Page) save() error {
	filename := p.Title +".txt"
	return ioutil.WriteFile(filename, p.Body, 0600)
}

func loadPage(title string) (*Page, error){
	fileName := "templates/" +title + ".html"
	body, err := ioutil.ReadFile(fileName)
	if err != nil {
		return nil, err
	}
	return &Page{Title: title, Body: body}, err
}

func view2Handler(w http.ResponseWriter, r *http.Request){
	
	switch r.Method {
	case "GET":		
		title := "view2/"+r.URL.Path[len("/view2/"):]
		p, err := loadPage("view2/"+title)
		fmt.Printf("called\n")
		if err != nil {
			p = &Page{Title: title}
		}
		renderTemplate(w, title,p)

	case "POST":
		fmt.Printf("reaceh here? ?\n")
		len := r.ContentLength
		body := make([]byte, len)
		r.Body.Read(body)
		data := Temp{}
		fmt.Println(string(body));
		err := json.Unmarshal([]byte(body), &data)
		if err != nil {
			fmt.Printf("json unmarshal err\n")
		}
		fmt.Printf("%s - %s - %s\n", data.Item1, data.Item2, data.Item3)
			
	default:
		fmt.Fprintf(w, "Sorry, only GET and POST methods are supported.")
	}
	
	
	
	
}

func viewHandler(w http.ResponseWriter, r *http.Request){
	title := "view/"+r.URL.Path[len("/view/"):]
	fmt.Printf(title+"\n")
	p, err := loadPage("view/"+title)
		if err != nil {
		p = &Page{Title: title}
	}
	renderTemplate(w, title, p)
}



func renderTemplate(w http.ResponseWriter, tmpl string, p *Page) {
	t, _ := template.ParseFiles("templates/" + tmpl +".html")
	t.Execute(w, p)
}


func main() {
    http.HandleFunc("/view/", viewHandler)
    http.HandleFunc("/view2/", view2Handler)
    log.Fatal(http.ListenAndServe(":8080", nil))
}