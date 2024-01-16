document.addEventListener("DOMContentLoaded", function() {
    const texts = [
        "Find the freshest produce in your area!",
        "Discover Local Delights! Fresh, Seasonal Goods Just a Click Away",
        // Add more prebuilt texts here
        "Connect with Your Food! Meet the Growers Transforming Your Meals!",
        "Savor the Harvest! Exclusive Access to Homegrown Treasures Awaits!",
        "From Farm to Fork! Experience the Journey of Freshness!",
        "Taste the Real Flavors! Support Local Farms Today!",
        "Join the Locavore Movement! Exceptional Produce at Your Fingertips!",
        "Freshness Alert! Your Source for the Best Local Eats!",
        "Cultivate Goodness! Find Organic and Artisanal Foods Near You!",
        "Harvest the Benefits! Nourish Your Community by Shopping Local!",
        "Elevate Your Plate! Premium Local Produce Just Around the Corner!"
    ];
  
    const images = [
      "1.png",
      "2.png",
      "3.png"
      // Add more prebuilt image filenames here
    ];
  
    function getRandomIndex(arrayLength) {
      return Math.floor(Math.random() * arrayLength);
    }

    function updateContent() {
        const textElement = document.getElementById("dynamic-text");
        const container = document.getElementById("container");
      
        // Get random text and image
        const newText = texts[getRandomIndex(texts.length)];
        const newImage = images[getRandomIndex(images.length)];
      
        // Update text with fade animation
        textElement.classList.remove("fade-in-text");
        container.classList.remove("fade-in-background");
      
        // Trigger reflow to restart the animation
        void textElement.offsetWidth;
        void container.offsetWidth;
      
        textElement.textContent = newText;
        container.style.backgroundImage = `url('${newImage}')`;
      
        textElement.classList.add("fade-in-text");
        container.classList.add("fade-in-background");
      }
  
    // Initial content update
    updateContent();
  
    // Update content every 5 seconds
    setInterval(updateContent, 10000);
  });
  
  