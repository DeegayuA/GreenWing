let currCity = "Kelaniya";
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

// Use an anonymous function to call getWeather on load
document.addEventListener('DOMContentLoaded', () => {
    getWeather();
});



