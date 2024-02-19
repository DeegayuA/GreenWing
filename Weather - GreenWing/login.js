document.getElementById('loginForm').addEventListener('submit', function(event) {
  event.preventDefault();
  var username = document.getElementById('username').value;
  var password = document.getElementById('password').value;

  var formData = new FormData();
  formData.append('username', username);
  formData.append('password', password);

  fetch('./login.php', {
    method: 'POST',
    body: formData
  })
  .then(response => {
    if (!response.ok) {
      response.text().then(text => { throw new Error(`HTTP error! status: ${response.status}, Body: ${text}`); });
    }
    // No need to parse JSON, as we are not returning JSON from PHP
    return response.text();
  })
  .then(data => {
    console.log('Success:', data);
    if (data.includes('Invalid credentials')) {
      var errorMessage = document.querySelector('.error');
      errorMessage.textContent = data;
      errorMessage.style.display = 'block';
    } else {
      // Redirect to preloader.html
      window.location.href = './preloader.html';
    }
  })
  .catch(error => {
    console.error('Error:', error);
  });
});
