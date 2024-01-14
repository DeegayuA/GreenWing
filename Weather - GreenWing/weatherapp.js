let currCity = "";
let units = "metric";

// Selectors
let city = document.querySelector(".weather__city");
let datetime = document.querySelector(".weather__datetime");
let weather__forecast = document.querySelector('.weather__forecast');
let weather__temperature = document.querySelector(".weather__temperature");
let weather__icon = document.querySelector(".weather__icon");
let weather__minmax = document.querySelector(".weather__minmax");
let weather__realfeel = document.querySelector('.weather__realfeel');
let weather__humidity = document.querySelector('.weather__humidity');
let weather__wind = document.querySelector('.weather__wind');
let weather__rain = document.querySelector('.weather__rain');
let weather__pressure = document.querySelector('.weather__pressure');

// search
document.querySelector(".weather__search").addEventListener('submit', e => {
    let search = document.querySelector(".weather__searchform");
    // prevent default action
    e.preventDefault();
    // change current city
    currCity = search.value;
    // get weather forecast 
    getWeather();
    // clear form
    search.value = "";
});

function convertTimeStamp(timestamp, timezone) {
    const convertTimezoneHours = Math.floor(timezone / 3600); // Whole hours
    const convertTimezoneMinutes = Math.floor((timezone % 3600) / 60); // Remaining minutes

    const date = new Date(timestamp * 1000);

    // Adjust the date with the time zone offset
    date.setHours(date.getHours() + convertTimezoneHours);
    date.setMinutes(date.getMinutes() + convertTimezoneMinutes);

    const options = {
        weekday: "long",
        day: "numeric",
        month: "long",
        year: "numeric",
        hour: "numeric",
        minute: "numeric",
        timeZone: "UTC", // Use UTC to prevent double adjustment
        hour12: false,
    };

    const formattedDate = date.toLocaleString("en-US", options);

    // Manually construct the time zone offset string

    return `${formattedDate}`;
}




// convert country code to name
function convertCountryCode(country) {
    let regionNames = new Intl.DisplayNames(["en"], { type: "region" });
    return regionNames.of(country);
}

function getWeather() {
    const API_KEY = '7b78e7d85f470c2529df86e060e5b836';

    fetch(`https://api.openweathermap.org/data/2.5/weather?q=${currCity}&appid=${API_KEY}&units=${units}`)
        .then(res => res.json())
        .then(data => {
            // console.log(data);
            city.innerHTML = `${data.name}, ${convertCountryCode(data.sys.country)}`;
            datetime.innerHTML = convertTimeStamp(data.dt, data.timezone); 
            weather__forecast.innerHTML = `<p>${data.weather[0].main}`;
            weather__temperature.innerHTML = `${data.main.temp.toFixed()}&#176`;
            weather__icon.innerHTML = `<img src="http://openweathermap.org/img/wn/${data.weather[0].icon}@4x.png" />`;
            weather__minmax.innerHTML = `<p>Min: ${data.main.temp_min.toFixed()}&#176;</p><p>Max: ${data.main.temp_max.toFixed()}&#176;</p>`;
            weather__realfeel.innerHTML = `${data.main.feels_like.toFixed()}&#176 C`;
            weather__humidity.innerHTML = `${data.main.humidity} %`;
            weather__pressure.innerHTML = `${data.main.pressure} hPa`;

            let windSpeed = data.wind.speed;
            if (units === "metric") {
                // Convert from m/s to km/h
                windSpeed = windSpeed * 3.6;
                weather__wind.innerHTML = `${windSpeed.toFixed(2)} km/h`;
            } else {
                // Imperial units, display in mph
                weather__wind.innerHTML = `${windSpeed} mph`;
            }

            // Add rain volume
            let rainVolume = 'No rain';
            if (data.rain) {
                rainVolume = data.rain['1h'] ? `${data.rain['1h']} mm/h` : (data.rain['3h'] ? `${data.rain['3h']} mm/h` : '0 mm/h');
            }
            weather__rain.innerHTML = `${rainVolume}`;

        });
}

// Function to get user's location
function getLocation() {
    if (navigator.geolocation) {
        navigator.geolocation.getCurrentPosition(showPosition, showError);
    } else {
        console.log("Geolocation is not supported by this browser.");
        getWeather(); // Call getWeather with the default city
    }
}

// Function to get location based on IP address
function getIPLocation() {
    fetch('https://ipapi.co/json/') // Replace with your chosen IP geolocation service URL
        .then(res => res.json())
        .then(data => {
            currCity = data.city || "Kelaniya"; // Use the city from IP location or default to "Kelaniya"
            getWeather();
        })
        .catch(err => {
            console.error("Error fetching IP-based location: ", err);
            currCity = "Kelaniya"; // Default to "Kelaniya" in case of any error
            getWeather();
        });
}

// Handle successful geolocation
function showPosition(position) {
    const API_KEY = '7b78e7d85f470c2529df86e060e5b836'; // Replace with your actual API key
    fetch(`https://api.openweathermap.org/geo/1.0/reverse?lat=${position.coords.latitude}&lon=${position.coords.longitude}&limit=1&appid=${API_KEY}`)
        .then(res => res.json())
        .then(data => {
            if (data.length > 0) {
                currCity = data[0].name;
            }
            getWeather(); // Call getWeatherwith the obtained city or default city
        })
        .catch(err => {
        console.error("Error with reverse geocoding: ", err);
        getWeather(); // Call getWeather with the default city in case of an error
        });
        }
        
        // Handle geolocation errors
        function showError(error) {
        switch(error.code) {
        case error.PERMISSION_DENIED:
        console.log("User denied the request for Geolocation.");
        getIPLocation(); 
        break;
        case error.POSITION_UNAVAILABLE:
        console.log("Location information is unavailable.");
        getIPLocation(); 
        break;
        case error.TIMEOUT:
        console.log("The request to get user location timed out.");
        getIPLocation(); 
        break;
        case error.UNKNOWN_ERROR:
        console.log("An unknown error occurred.");
        getIPLocation(); 
        break;
        }
        getWeather(); // Call getWeather with the default city if there's an error
        }
        
        // Event listener for DOMContentLoaded
        document.addEventListener('DOMContentLoaded', () => {
        getLocation(); // Try to get user's location on page load
        });


