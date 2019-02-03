// https://stackoverflow.com/questions/2928827/access-php-var-from-external-javascript-file

//console.log($("#sourceAcct").val())
var myAcct = "2325235"
window.onload=function() {
    var inputVariables = document.getElementsByTagName("input")
    inputVariables[3].onclick = function() {
        alert("clicked!")
        inputVariables[1].value = myAcct
    }
}

