function changeLink(link) {
	window.location.href = link;
}

const button_404 = document.getElementById("404_button");
const button_form = document.getElementById("form_button");

button_404.addEventListener("click", () => {
	changeLink("/404");
});

button_form.addEventListener("click", () => {
	changeLink("/form");
});
