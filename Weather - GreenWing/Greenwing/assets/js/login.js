document.getElementById('loginForm').addEventListener('submit', function(event) {
  event.preventDefault();
  var username = document.getElementById('username').value;
  var password = document.getElementById('password').value;
  // If you have an email field, you can retrieve it like this:
  // var email = document.getElementById('email').value;

  var formData = new FormData();
  formData.append('username', username);
  formData.append('password', password);
  // formData.append('email', email); // Include this if you're also sending the email to login

  fetch('../assets/php/login.php', {
    method: 'POST',
    body: formData
  })
  .then(response => {
    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }
    return response.json();
  })
  .then(data => {
    console.log('Success:', data); // Check the data object in the browser console
    if (data.success) {
      window.location.href = './page/home.html';
    } else {
      var errorMessage = document.querySelector('.error');
      errorMessage.textContent = data.message;
      errorMessage.style.display = 'block';
    }
  })
  .catch((error) => {
    console.error('Error:', error);
  });
});
