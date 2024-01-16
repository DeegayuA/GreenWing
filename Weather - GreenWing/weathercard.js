// Declare currCity variable outside of functions
let currCity = "";

// Function to get user's location
function getLocation() {
  if (navigator.geolocation) {
    navigator.geolocation.getCurrentPosition(showPosition, showError);
  } else {
    console.log("Geolocation is not supported by this browser.");
    getIPLocation();
  }
}

// Function to get location based on IP address
function getIPLocation() {
  fetch("https://ipapi.co/json/") // Replace with your chosen IP geolocation service URL
    .then((res) => res.json())
    .then((data) => {
      currCity = data.city || "Kelaniya"; // Use the city from IP location or default to "Kelaniya"
      getWeather();
    })
    .catch((err) => {
      console.error("Error fetching IP-based location: ", err);
      currCity = "Kelaniya"; // Default to "Kelaniya" in case of any error
      getWeather();
    });
}

// Handle successful geolocation
function showPosition(position) {
  const API_KEY = "7b78e7d85f470c2529df86e060e5b836"; // Replace with your actual API key
  fetch(
    `https://api.openweathermap.org/geo/1.0/reverse?lat=${position.coords.latitude}&lon=${position.coords.longitude}&limit=1&appid=${API_KEY}`
  )
    .then((res) => res.json())
    .then((data) => {
      if (data.length > 0) {
        currCity = data[0].name;
      }
      getWeather(); // Call getWeather with the obtained city or default city
    })
    .catch((err) => {
      console.error("Error with reverse geocoding: ", err);
      getWeather(); // Call getWeather with the default city in case of an error
    });
}

// Handle geolocation errors
function showError(error) {
  switch (error.code) {
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
document.addEventListener("DOMContentLoaded", () => {
  getLocation(); // Try to get user's location on page load

  // Now that the DOM is fully loaded, add the event listener for the search form
  document.querySelector(".weather__search").addEventListener("submit", (e) => {
    let search = document.querySelector(".weather__search-input"); // Change to search input
    // prevent default action
    e.preventDefault();
    // change current city
    currCity = search.value;
    // get weather forecast
    getWeather();
    // clear form
    search.value = "";
  });
});

// Function to convert country code to name
function convertCountryCode(country) {
  let regionNames = new Intl.DisplayNames(["en"], { type: "region" });
  return regionNames.of(country);
}

// Function to convert timestamp to formatted date and time with timezone
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

function getWeather() {
  /* TIME & DATE */
  const apiKey = "7b78e7d85f470c2529df86e060e5b836";
  const url = `https://api.openweathermap.org/data/2.5/weather?q=${currCity}&appid=${apiKey}&units=metric`;
  const forecastUrl = `https://api.openweathermap.org/data/2.5/forecast?q=${currCity}&appid=${apiKey}&units=metric`;


  fetch(url)
    .then((response) => response.json())
    .then((data) => {
      if (data.main && data.name && data.weather && data.weather.length > 0) {
        const { main, name, weather } = data;
        document.querySelector(".card__content__city").innerText = name;
        document.querySelector(".card__temp--current").innerText = Math.round(
          main.temp
        );
        document.querySelector("#min-temp").innerText = Math.round(
          main.temp_min
        );
        document.querySelector("#max-temp").innerText = Math.round(
          main.temp_max
        );

        let iconWeather;
        const weatherBg = document.querySelector(".card__image");
        switch (weather[0].main) {
          case "Rain":
            iconWeather = "card__image--rain";
            break;
          case "Clear":
            iconWeather = "";
            break;
          case "Snow":
            iconWeather = "fa-snowflake";
            break;
          case "Clouds":
            iconWeather = "card__image--cloudy";
            break;
          case "Drizzle":
            iconWeather = "card__image--drizzle";
            break;
          case "Thunderstorm":
            iconWeather = "card__image--rain";
            break;
          default:
            iconWeather = "";
        }
        weatherBg.className = "card__image"; // Remove previous classes
        if (iconWeather) {
          weatherBg.classList.add(iconWeather);
        }

        // Get user's time zone offset in seconds
        const timeZoneOffset = data.timezone;

        // Get the timestamp and convert it to a formatted date and time with timezone
        const timestamp = data.dt;
        const formattedTime = convertTimeStamp(timestamp, timeZoneOffset);

        // Write date and time to HTML
        document.querySelector(".details__time").innerText = formattedTime;
      } else {
        console.error(
          "Error: Invalid data received from the API. Data :" +
            JSON.stringify(data)
        );
      }
    })
    .catch((error) => {
      console.error("Error:", error);
    });

  // Fetch the 5-day forecast
  fetch(forecastUrl)
        .then(response => response.json())
        .then(data => {
            if (data.list && data.list.length > 0) {
                for (let i = 1; i <= 3; i++) {
                    const forecastIndex = i * 8 - 1; 
                    const dayForecast = data.list[forecastIndex];
                    const date = new Date(dayForecast.dt_txt);

                    // Update HTML for each day
                    document.querySelector(`.day${i + 1} .weather-card-min__info__condition`).innerText = dayForecast.weather[0].main;
                    document.querySelector(`.day${i + 1} .weather-card-min__weather__temp`).innerText = `${Math.round(dayForecast.main.temp)}° C`;
                    document.querySelector(`.day${i + 1} .weather-card-min__info__date2`).innerText = date.toLocaleDateString("en-US", { weekday: "long", month: "short", day: "numeric" });

                    // Set icon for each day in the forecast
                    const forecastIconUrl = `http://openweathermap.org/img/wn/${dayForecast.weather[0].icon}@4x.png`;
document.querySelector(`.day${i + 1} .weather-card-min__icon`).innerHTML = `<img src="${forecastIconUrl}" class="weather-icon" />`;
                }
            }
        })
        .catch(error => console.error("Error:", error));
}
