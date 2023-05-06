const express = require("express");
const dotenv = require("dotenv").config();
const { MongoClient } = require("mongodb");
const dbURL = require("./config/dbConnection");

const client = new MongoClient(dbURL, { useNewUrlParser: true, useUnifiedTopology: true });

client.connect();
console.log("Connected successfully to MongoDB");
const db = client.db();

const app = express();
const port = process.env.PORT || 7070;

app.use(express.json());

app.get("/updateData", function (req, res) {
    var data = req.query.data || null;
    var mac = req.query.mac || null;

    if (data && mac) {
        try {
        var finalData = JSON.parse(data);
        finalData.time = new Date().getTime();
        finalData.mac = mac;

        console.log(finalData);
        
        db.collection("data").insertOne(finalData, function (e, r) {
            res.send(1);
        });

        res.status(201).json(finalData);
        } catch (e) {
        console.log(e);
        }
    }
});

app.listen(port, () => {
    console.log(`Server running on port ${port}`);
});
