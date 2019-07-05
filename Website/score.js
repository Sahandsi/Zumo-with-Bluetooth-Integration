var mqtt;                                                                                                                               // declaring variable
var reconnectTimeout = 2000;                                                                                                            // declaring after how many microseconds it should try to reconnect if the connection fails
var host="broker.mqttdashboard.com";                                                                                                    // defining the MQTT Broker
var port=8000;                                                                                                                          // defining the PORT that should be connected to
var zumos=[];                                                                                                                           // defining an empty array that will hold the zumo object

function onMessageArrived(msg){                                                                                                         // function name
    let incoming = msg.payloadString;                                                                                                   // define incoming MQTT messages as the incoming variables
    console.log(incoming);                                                                                                              // log each incoming message in the Browser console

    if(incoming.indexOf('|') > -1){                                                                                                     // If the incoming message has a '|' character inside it
        let explode = incoming.split('|');                                                                                              // Split the string where the delimiter is '|'. Store them in the explode array
        let health = explode[1].split('=')[1];                                                                                          // Split the explode string where the index of explode is 1, where the delimiter is '=' and store index 1 in the health variable
        console.log(explode[0]);                                                                                                        // Log the explode array in the broweser console
        console.log(health);                                                                                                            // Log the health of the zumo in the broswer console

        let zumo = {};                                                                                                                  // define zumo as an object
        zumo.name=explode[0];                                                                                                           // define the zumo name
        zumo.health = health;                                                                                                           //  define the zumo health

        let findZumo = zumos.filter(e => e.name === zumo.name);                                                                         // Check if the zumos array has a zumo with the same name as the new zumo object name

        if (findZumo.length > 0) {                                                                                                      // If the zumo exists in the zumos array
            findZumo[0].health = zumo.health;                                                                                           // Set the new health for that zumo
        }else{                                                                                                                          // If the new zumo object does not exist
            zumos.push(zumo);                                                                                                           // Add the new zumo object to the zumos array
        }

        for(i = 0; i < zumos.length; ++i){                                                                                              // For each zumo that exists inside the zumos array
            if(i == 0){                                                                                                                 // For the first zumo
                document.getElementById("homecomming-team-name").innerHTML = zumos[i].name;                                             // Inject Zumo name to HTML
                document.getElementById("home-team-score").innerHTML = zumos[i].health;                                                 // Inject Zumo health to HTML 
                document.getElementById("home-team-logo").style.display = "inline";                                                     // Display the Zumo image in HTML

                if(zumos[i].health < 60 && zumos[i].health >= 30 ){                                                                     // If the zumo health is between 60 and 30
                    document.getElementById("home-team-score").style.color = "Orange";                                                  // the colour of the health is Orange
                }else if(zumos[i].health < 30 ){                                                                                        // If the Zumo health is less than 30
                    document.getElementById("home-team-score").style.color = "Red";                                                     // colour of the health is red
                }else{                                                                                                                  // if the health is above 60
                    document.getElementById("home-team-score").style.color = "Green";                                                   // colour of the health is green
                }

                if(zumos[i].health == 0 || zumos[i].health < 0){                                                                        // if the zumo health is 0 or less than 0
                    document.getElementById("connectionMessage").innerHTML = zumos[i+1].name + " WON THE GAME";                         // Inject victory message in HTML
                    document.getElementById("victoryDance").style.display = "inline";                                                   // Display the confetti in HTML
                }
            }else if(i == 1){                                                                                                           // For the second zumo
                document.getElementById("away-team-name").innerHTML = zumos[i].name;                                                    // Inject Zumo name to HTML
                document.getElementById("away-team-score").innerHTML = zumos[i].health;                                                 // Inject Zumo health to HTML
                document.getElementById("away-team-logo").style.display = "inline";                                                     // Display the zumo logo

                if(zumos[i].health < 60 && zumos[i].health >= 30){                                                                      // If the health of the zumo is between 60 and 30
                    document.getElementById("away-team-score").style.color = "Orange";                                                  // Colour of the health is Orange
                }else if(zumos[i].health < 30 ){                                                                                        // If the Zumo health is less than 30
                    document.getElementById("away-team-score").style.color = "Red";                                                     // COlour of the health is red
                }else{                                                                                                                  // If the health of the zumo is more than 60
                    document.getElementById("away-team-score").style.color = "Green";                                                   // Colour of the health is Green
                }

                if(zumos[i].health == 0 || zumos[i].health < 0){                                                                        // If the Zumo health is 0 or less
                    document.getElementById("connectionMessage").innerHTML = zumos[i-1].name + " WON THE GAME";                         // Inject victory message to HTML
                    document.getElementById("victoryDance").style.display = "inline";                                                   // Display Confetti in HTML
                }
            }
        }
    }else{                                                                                                                              // If the incoming message does not have a '|'
        document.getElementById("connectionMessage").innerHTML = msg.payloadString;                                                     // Inject the incoming message to HTML
    }
}

function onFailure(message) {                                                                                                           // Function that is run when the MQTT cannot connect
    console.log("Connection Attempt to Host "+host+"Failed");                                                                           // Log the attempt to browser console
    setTimeout(MQTTconnect, reconnectTimeout);                                                                                          // try to run the function MQTTconnect every 2000 milliseconds as defined on line 2
    document.getElementById("connectionMessage").style.color = "red";                                                                   // Set the colour of the message red in HTML
    document.getElementById("connectionMessage").innerHTML = "Connection to host: " + host + " failed.";                                // Inject Connection Failure to HTML
}

function onConnect() {                                                                                                                  // Run on succesfull MQTT Connection
    console.log("Connected ");                                                                                                          // Log the connection sucess to browser console
    let topic = "zumo/mcu/data";                                                                                                          // Defining the topic that should be subscribed to
    mqtt.subscribe(topic);                                                                                                              // MQTT Subscribing to the above topic
    message = new Paho.MQTT.Message("Client Connected Sucessfully");                                                                    // Construct message to send to MQTT Broker
    message.destinationName = topic;                                                                                                    // Topic that the message should be sent to 
    mqtt.send(message);                                                                                                                 // Sending the message
}

function MQTTconnect() {                                                                                                                // Function that is run when the HTML is loaded
    console.log("connecting to "+ host +" "+ port);                                                                                     // Log the host and port that the MQTT is trying to connect to in the broswer console
    mqtt = new Paho.MQTT.Client(host,port,"clientjs");                                                                                  // Construct the Connection variable
    var options = {                                                                                                                     // define option object
            timeout: 3,                                                                                                                 // define timeout
            userName : "",                                                                                                              // define username
            password : "",                                                                                                              // define password
            onSuccess: onConnect,                                                                                                       // define what to do after successfull connection
            onFailure: onFailure,                                                                                                       // define what to do after failed connection
        };  
    mqtt.onMessageArrived = onMessageArrived;                                                                                           // On a message receive, run the onMessageArrived function
    mqtt.connect(options);                                                                                                              // Attempt to connect to the MQTT Broker
}