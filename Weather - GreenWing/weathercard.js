let currCity = "";
let allDescriptions = [];

function getLocation() {
  if (navigator.geolocation) {
    navigator.geolocation.getCurrentPosition(showPosition, showError);
  } else {
    console.log("Geolocation is not supported by this browser.");
    getIPLocation();
  }
}

function getIPLocation() {
  fetch("https://ipapi.co/json/")
    .then((res) => res.json())
    .then((data) => {
      currCity = data.city || "Kelaniya";
      getWeather();
    })
    .catch(handleLocationError);
}

function handleLocationError(err) {
  console.error("Error fetching location: ", err);
  currCity = "Kelaniya";
  getWeather();
}

function showPosition(position) {
  const API_KEY = "7b78e7d85f470c2529df86e060e5b836";
  fetchWeatherByCoords(position.coords.latitude, position.coords.longitude, API_KEY);
}

function fetchWeatherByCoords(latitude, longitude, apiKey) {
  fetch(`https://api.openweathermap.org/geo/1.0/reverse?lat=${latitude}&lon=${longitude}&limit=1&appid=${apiKey}`)
    .then((res) => res.json())
    .then((data) => {
      currCity = data.length > 0 ? data[0].name : currCity;
      getWeather();
    })
    .catch((err) => {
      console.error("Error with reverse geocoding: ", err);
      getWeather();
    });
}

function showError(error) {
  console.log("Geolocation error:", error.message);
  getIPLocation();
}

document.addEventListener("DOMContentLoaded", () => {
  getLocation();
  document.querySelector(".weather__search").addEventListener("submit", handleSearch);
  getWeather();
  updateIrrigationStatus(temp, minTemp, rain, wind);
});

function handleSearch(e) {
  e.preventDefault();
  currCity = document.querySelector(".weather__search-input").value;
  getWeather();
  document.querySelector(".weather__search-input").value = "";
}

function convertCountryCode(country) {
  let regionNames = new Intl.DisplayNames(["en"], { type: "region" });
  return regionNames.of(country);
}

function convertTimeStamp(timestamp, timezone) {
  const date = new Date(timestamp * 1000);
  const offsetMilliseconds = date.getTimezoneOffset() * 60 * 1000;
  const localTime = date.getTime() + offsetMilliseconds + timezone * 1000;

  const options = {
    weekday: "long",
    day: "numeric",
    month: "long",
    year: "numeric",
    hour: "numeric",
    minute: "numeric",
    timeZone: "UTC",
    hour12: false,
  };

  return new Date(localTime).toLocaleString("en-US", options);
}

function handleSnowAnimation() {
  const weatherDiv = document.querySelector(".card__image");
  let i = 0;

  function addSnowflake() {
    if (i < 100) {
      const snowflake = createWeatherElement("div", "weather__snowflake");
      setRandomPosition(snowflake);
      setRandomSize(snowflake, 10, 10);
      setRandomAnimationDuration(snowflake, 6, 10);

      weatherDiv.appendChild(snowflake);
      i++;
      setTimeout(addSnowflake, 200);
    }
  }

  addSnowflake();
}
function handlePrecipitationAnimation(type) {
  const weatherDiv = document.querySelector(".card__image");
  const rainContainer = document.querySelector(".weather__rain-container");
  let totalDrops = type === "Rain" ? 100 : 50;

  // // Remove existing rain elements
  // removeWeatherAnimations(".weather__rain");


  function addDrop(i) {
    if (i >= totalDrops) return;

    const drop = createWeatherElement("div", "weather__rain");
    setRandomPosition(drop);

    // Set the angle of the raindrop's path
    const angle = -20; // Adjust the angle as needed
    drop.style.transform = `rotate(${angle}deg)`;

    if (type === "Rain") {
      // Use rectangular raindrop
      setRandomSize(drop, 2, 4, 20, 40);
      setRandomAnimationDuration(drop, 0.5, 1);
    } else {
      setRandomSize(drop, 2, 4, 10, 15);
      setRandomAnimationDuration(drop, 2, 3);
    }

    rainContainer.appendChild(drop);

    // Remove the oldest drop if the total exceeds the limit
    if (rainContainer.children.length > totalDrops) {
      rainContainer.removeChild(rainContainer.children[0]);
    }

    setTimeout(() => addDrop(i + 1), 50);
  }

  addDrop(0);
}




function handleThunderstormAnimation() {
  const weatherDiv = document.querySelector(".card__image");

  function addLightningFlash() {
    const lightningFlash = createWeatherElement("div", "weather__lightning-flash");
    weatherDiv.appendChild(lightningFlash);

    setTimeout(() => {
      lightningFlash.remove();
    }, 200);
  }

  function addRainDuringThunderstorm() {
    handlePrecipitationAnimation("Rain");
  }

  function startThunderstormAnimation() {
    addLightningFlash();
    addRainDuringThunderstorm(0);
    setTimeout(startThunderstormAnimation, 3000 + Math.random() * 3000);
  }

  startThunderstormAnimation();
}


function createWeatherElement(tagName, className) {
  const element = document.createElement(tagName);
  element.className = className;
  return element;
}

function setRandomPosition(element) {
  element.style.left = Math.random() * 100 + "%";
  element.style.top = -10 + Math.random() * 20 + "%";
}

function setRandomSize(element, minWidth, maxWidth, minHeight, maxHeight) {
  const width = Math.random() * (maxWidth - minWidth) + minWidth;
  const height = Math.random() * (maxHeight - minHeight) + minHeight;
  element.style.width = `${width}px`;
  element.style.height = `${height}px`;
}

function setRandomAnimationDuration(element, minDuration, maxDuration) {
  const duration = Math.random() * (maxDuration - minDuration) + minDuration;
  element.style.animationDuration = `${duration}s`;
}

function getWeather() {
  const apiKey = "7b78e7d85f470c2529df86e060e5b836";
  const url = `https://api.openweathermap.org/data/2.5/weather?q=${currCity}&appid=${apiKey}&units=metric`;
  const forecastUrl = `https://api.openweathermap.org/data/2.5/forecast?q=${currCity}&appid=${apiKey}&units=metric`;

  fetch(url)
    .then((response) => response.json())
    .then(handleWeatherData)
    .catch((error) => {
      console.error("Error fetching weather data:", error);
    });

  fetch(forecastUrl)
    .then((response) => response.json())
    .then(handleForecastData)
    .catch((error) => {
      console.error("Error fetching forecast data:", error);
    });
}

function handleWeatherData(data) {
  if (data.main && data.name && data.weather && data.weather.length > 0 && data.wind && data.wind.speed > 0) {
    const { main, name, weather, dt, timezone, rain, wind } = data;
    allDescriptions = data.weather.map((w) => w.description);
    updateWeatherDisplay(name, main.temp, main.temp_min, main.temp_max, weather[0].main, dt, timezone);
    updateWeatherBackground(weather[0].main);
    updateIrrigationStatus(main.temp, main.temp_min, rain, wind);
  } else {
    console.error("Error: Invalid data received from the API. Data:", JSON.stringify(data));
  }
}


function updateWeatherDisplay(city, temp, minTemp, maxTemp, condition, timestamp, timezone) {
  document.querySelector(".card__content__city").innerText = city;
  document.querySelector(".card__temp--current").innerText = Math.round(temp);
  document.querySelector("#min-temp").innerText = Math.round(minTemp);
  document.querySelector("#max-temp").innerText = Math.round(maxTemp);

  const formattedTime = convertTimeStamp(timestamp, timezone);
  document.querySelector(".details__time").innerText = formattedTime;
}

function updateWeatherBackground(weatherCondition) {
  const weatherBg = document.querySelector(".card__image");
  weatherBg.className = "card__image";

  // weatherCondition = "Rain"

  switch (weatherCondition) {
    case "Rain":
      weatherBg.classList.add("card__image--rain");
      handlePrecipitationAnimation("Rain");
      break;
    case "Drizzle":
      weatherBg.classList.add("card__image--drizzle");
      handlePrecipitationAnimation("Drizzle");
      break;
    case "Clear":
      const clouds = document.querySelectorAll(".weather__cloud--big, .weather__cloud");
      clouds.forEach(cloud => {
        cloud.style.display = 'none'; // or cloud.remove();
      });
      break;
    case "Snow":
      weatherBg.classList.add("card__image--snow");
      handleSnowAnimation();
      break;
    case "Clouds":
      weatherBg.classList.add("card__image--cloudy");
      break;
    case "Thunderstorm":
      weatherBg.classList.add("card__image--rain");
      handleThunderstormAnimation();
      break;
    default:
    // No additional class for other weather conditions
  }

  // Remove rain and snow animations if it's not "Rain", "Drizzle", or "Snow"
  if (["Rain", "Drizzle", "Snow", "Thunderstorm"].indexOf(weatherCondition) === -1) {
    removeWeatherAnimations(".weather__rain");
    removeWeatherAnimations(".weather__snowflake");
  }
}

function removeWeatherAnimations(selector) {
  const elements = document.querySelectorAll(selector);
  elements.forEach((element) => {
    element.remove();
  });
}

function handleForecastData(data) {
  if (data.list && data.list.length > 0) {
    for (let i = 1; i <= 4; i++) {
      const forecastIndex = i * 8 - 1;
      const dayForecast = data.list[forecastIndex];
      const date = new Date(dayForecast.dt_txt);

      updateForecastCard(i, dayForecast.weather[0].main, dayForecast.main.temp, date);
      updateForecastIcon(i, dayForecast.weather[0].icon);
    }
  }
}

function updateForecastCard(dayIndex, condition, temperature, date) {
  const dayElement = document.querySelector(`.day${dayIndex + 1}`);
  if (dayElement) {
    dayElement.querySelector(".weather-card-min__info__condition").innerText = condition;
    dayElement.querySelector(".weather-card-min__weather__temp").innerText = `${Math.round(temperature)}° C`;
    dayElement.querySelector(".weather-card-min__info__date2").innerText = date.toLocaleDateString("en-US", {
      weekday: "long",
      month: "short",
      day: "numeric",
    });
  }
}

function updateForecastIcon(dayIndex, iconCode) {
  const forecastIconUrl = `http://openweathermap.org/img/wn/${iconCode}@4x.png`;
  const iconElement = document.querySelector(`.day${dayIndex + 1} .weather-card-min__icon`);
  if (iconElement) {
    iconElement.innerHTML = `<img src="${forecastIconUrl}" class="weather-icon" />`;
  }
}

var validatedState;
var irrigationStatus;

function updateIrrigationStatus(temp, minTemp, rain, wind) {
  var irrigationStatusElement = document.getElementById("irrigationStatus");

  if (rain && rain["1h"]) {
    if (rain["1h"] < 1.0) {
      irrigationStatusElement.textContent = "Light rain detected (" + rain["1h"] + " mm/h). Fly with caution.";
    } else if (rain["1h"] >= 1 && rain["1h"] <= 7.5) {
      irrigationStatusElement.textContent = "Moderate rain detected (" + rain["1h"] + " mm/h). Use caution when flying drone.";
    } else if (rain["1h"] > 7.5 && rain["1h"] <= 50) {
      irrigationStatusElement.textContent = "Heavy rain detected (" + rain["1h"] + " mm/h). Avoid irrigation activities.";
    } else if (rain["1h"] > 50 && rain["1h"] <= 100) {
      irrigationStatusElement.textContent = "Very heavy rain detected (" + rain["1h"] + " mm/h). Avoid outdoor operations.";
    } else {
      irrigationStatusElement.textContent = "Extreme rain detected (" + rain["1h"] + " mm/h). Cease all outdoor activities.";
    }
    irrigationStatus = irrigationStatusElement.textContent;
    validatedState = false;
  } else if (wind.speed > 20) {
    irrigationStatusElement.textContent = "Strong winds detected (" + wind.speed + " m/s). Avoid drone operation.";
    irrigationStatus = irrigationStatusElement.textContent;
    validatedState = false;
  } else if (temp <= minTemp - 1 && temp >= minTemp + 1) {
    irrigationStatusElement.textContent = "Temperature (" + temp + "°C) is outside today's acceptable range. Adjust irrigation plans.";
    irrigationStatus = irrigationStatusElement.textContent;
    validatedState = false;
  } else {
    irrigationStatusElement.textContent = "Optimal conditions for watering. Now, Temperature is " + temp + "°C, Today's Min Temp is " + minTemp + "°C, Wind Speed is " + wind.speed + " m/s." + (rain && rain["1h"] ? " Rain is " + rain["1h"] + " mm/h" : " No rain detected");
    irrigationStatus = irrigationStatusElement.textContent;
    validatedState = true;
  }
}



// Function to validate and redirect
function validateAndRedirect() {
  // validatedState = updateIrrigationStatus(temp, minTemp);
  if (validatedState === true) {
    // Show specific warning message
    alert("Perfect Time for Irrigation: " + irrigationStatus);

    // Set a timeout to redirect after 3 seconds (adjust as needed)
    setTimeout(function () {
      window.location.href = "https://greenwing.netlify.app/";
    }, 3000);
  } else {
    // Show alert indicating the temperature is not within the acceptable range
    alert("Warning: " + irrigationStatus);

    // Redirect immediately
    window.location.href = "https://greenwing.netlify.app/";
  }
}

// Add event listener for DOMContentLoaded to ensure the script executes after the DOM is loaded
document.addEventListener("DOMContentLoaded", function () {
  // Attach onclick event to the button
  document.getElementById("startButton").onclick = validateAndRedirect;
});
