// script.js -----
function changeLink(link) {
	window.location.href = link;
}

const button = document.getElementById("404_button");

button.addEventListener("click", () => {
	changeLink("/404");
});
