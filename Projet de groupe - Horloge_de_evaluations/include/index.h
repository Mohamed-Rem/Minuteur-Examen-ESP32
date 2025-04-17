#include <Arduino.h>

String htmlPage = R"rawliteral(
          
    <!DOCTYPE html>
        <html>
            <head>
                <meta charset='utf-8'>
                <meta charset="UTF-8">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <link href="https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css" rel="stylesheet">
                <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">
                    
                <title>Minuterie d'Examen</title>

                <style>
                    .container {
                        height: auto;
                        display: flex;
                        flex-direction: column;
                        justify-content: center;
                        align-items: center;
                    }
                    .main {
                        width: 60vw;
                        border-radius: 6px;
                    }

                    .header h1 {
                        border-radius: 6px;
                        padding: 20px 40px;
                        text-transform: capitalize;
                        background-color: #111;
                        color: #fff;
                        text-align: center;
                    }

                    .body .time {
                        padding: 10px 40px;
                        font-size: 50px;
                        text-align: center;
                        font-weight: 700;
                        margin: 30px 0px;
                    }

                    .body .time #seconds {
                        margin-left: 15px;
                        text-align: center;
                        font-size: 18px;
                        vertical-align: middle;
                        margin-bottom: 0rem;
                        color: #444;
                    }

                    .body .setTime,
                    .body .setAlert {
                        padding: 0px 40px 30px;
                        text-align: center;
                    }

                    .body .setTime label,
                    .body .setAlert label {
                        font-size: 15px;
                        vertical-align: middle;
                        font-weight: 500;
                    }

                    .body .setTime input,
                    .body .setAlert input {
                        text-align: center;
                        vertical-align: middle;
                        margin-right: 3rem;
                    }

                    .body .setTime button,
                    .body .setAlert button {
                        padding: 4px 12px;
                    }

                    .body .alerts {
                        padding: 0px 20px;
                        margin-bottom: 20px;
                    }

                    .editBtns {
                        text-align: center;
                        padding: 0px 40px 30px;
                    }

                    .lock {
                        display: block;
                        padding: 10px;
                        font-size: 20px;
                        text-align: center;
                    }

                    #lock {
                        cursor: pointer;
                    }

                </style>
                
            </head>
            
            <body>
                <div class="container">
                
                    <div class="main card">
                        
                        <div class="header"><h1>minuterie examen</h1></div>
                        
                        <div class="body">

                            <div class="time">
                                <p id='timer'>0 minutes</p>
                                <p id='seconds'>0 seconds</p>
                            </div>
        
                            <div class="setTime">
                                <label for="minuteInput">Temps (en minutes) :</label>
                                <input type="number" id="minuteInput"  name="minutes" min="1" max="60" value="5">
                                <button class="btn btn-dark" onclick="setTime()">Définir le temps</button>
                            </div>
        
                            <div class="setAlert">
                                <label for="alertInput">Alerts (en minutes) :</label>
                                <input type="number" id="alertInput"  name="alert" min="1" max="60" value="0">
                                <button class="btn btn-dark" onclick="setalert()">Définir le temps</button>
                            </div>

                            <div class="alerts">
                                <ul class="list-group" id="alerts_list"></ul>
                            </div>
    
                            <div class="editBtns">
                                <button  class="btn btn-success" onclick="setStates('start')">Démarrer</button>
                                <button class="btn btn-warning" onclick="setStates('stop')">Arrêter</button>
                                <button class="btn btn-danger" onclick="setStates('reset')">Réinitialiser</button>
                            </div>
                            <p class="lock"> System State : <i id = "lock" class="fa fa-unlock-alt" aria-hidden="true"></i> </p>
                        </div>
                    </div>
                </div>
            </body>
    
            <script>
                
                let remainingMinutes = 0;
                let remainingSeconds = 0;
                let syncInterval = null;
                let localInterval = null;

                function syncWithESP32() {
                    fetch('/data')
                    .then(response => response.json())
                    .then(data => {
                        remainingMinutes = data.minutes;
                        remainingSeconds = data.seconds;
                        console.log("Synchronisé avec l'ESP32 : " + remainingMinutes + "m " + remainingSeconds + "s");

                        document.getElementById('timer').innerText = remainingMinutes + ' minutes';
                        document.getElementById('seconds').innerText = remainingSeconds + ' seconds';
                    });
                }

                function updateLocalTimer() {
                    if (remainingSeconds === 0) {
                        if (remainingMinutes > 0) {
                            remainingMinutes--;
                            remainingSeconds = 59;
                        }
                    } else {
                        remainingSeconds--;
                    }

                    document.getElementById('timer').innerText = remainingMinutes + ' minutes';
                    document.getElementById('seconds').innerText = remainingSeconds + ' seconds';
                }

                function startTimers() {
                    if (!syncInterval) {
                        syncWithESP32(); // Sync immédiat
                        syncInterval = setInterval(syncWithESP32, 60000); // Toutes les minutes
                    }

                    if (!localInterval) {
                        localInterval = setInterval(updateLocalTimer, 1000); // Mise à jour locale chaque seconde
                    }
                }

                function stopTimers() {
                    clearInterval(syncInterval);
                    clearInterval(localInterval);
                    syncInterval = null;
                    localInterval = null;
                }

                function setStates(state) {
                    console.log(state);
                    fetch('/' + state);

                    if (state === 'start') {
                        startTimers();
                    } else if (state === 'stop') {
                        stopTimers();
                    } else if (state === 'reset') {
                        stopTimers();
                        remainingMinutes = 0;
                        remainingSeconds = 0;
                        document.getElementById('timer').innerText = '0 minutes';
                        document.getElementById('seconds').innerText = '0 seconds';
                        document.getElementById('alerts_list').innerHTML = '';
                    }
                }

                // Fonction pour envoyer la durée de la minuterie
                function setTime() {
                    var minutes = document.getElementById('minuteInput').value;
                    fetch('/settime?minutes=' + minutes)
                    .then(response => response.json())
                    .then(data => {
                        document.getElementById('timer').innerText = data.minutes +' minutes';
                        console.log("Time set to: " + data.minutes + " minutes");
                    });
                }
                
                function setalert() {
                    var alerts = document.getElementById('alertInput').value;
                    fetch('/settime?alerts=' + alerts)
                    .then(response => response.json())
                    .then(data => {
                        console.log("Alert set to: " + data.alerts + " alerts");
                        document.getElementById('alerts_list').insertAdjacentHTML('beforeend', '<li class="list-group-item list-group-item-info">- Une alerte est programmée ' + data.alerts + ' minutes avant la fin.</li>');
                    });
                }
                
                
                const icon = document.getElementById("lock");
                const buttons = document.querySelectorAll("button");

                icon.addEventListener("click", () => {
                if (icon.classList.contains("fa-lock")) {
                    icon.classList.remove("fa-lock");
                    icon.classList.add("fa-unlock-alt");
                     buttons.forEach(btn => {
                        btn.disabled = false;
                    });

                } else {
                    icon.classList.remove("fa-unlock-alt");
                    icon.classList.add("fa-lock");
                     buttons.forEach(btn => {
                      btn.disabled = true;
                    });
                }
                });
                        
            </script>
            
        </html>
    
)rawliteral";