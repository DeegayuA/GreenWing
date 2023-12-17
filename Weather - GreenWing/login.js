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
    return response.json();
  })
  .then(data => {
    console.log('Success:', data);
    if (data.success) {
      window.location.href = './preloader.html';
    } else {
      var errorMessage = document.querySelector('.error');
      errorMessage.textContent = data.message;
      errorMessage.style.display = 'block';
    }
  })
  .catch(error => {
    console.error('Error:', error);
  });
});
