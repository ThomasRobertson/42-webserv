function changeLink(link) {
	window.location.href = link;
}

const button_404 = document.getElementById("404_button");
const button_form = document.getElementById("form_button");

button_404.addEventListener("click", () => {
	changeLink("/404");
});

button_form.addEventListener("click", () => {
	changeLink("/working_form");
});

alt_form_button.addEventListener("click", () => {
	changeLink("/working_form/");
});

invalid_form_button.addEventListener("click", () => {
	changeLink("/form");
});

list_dir_button.addEventListener("click", () => {
	changeLink("/list_dir_test");
});

alt_list_dir_button.addEventListener("click", () => {
	changeLink("/list_dir_test/");
});

invalid_list_dir_button.addEventListener("click", () => {
	changeLink("/no_list_dir");
});

cgi_button.addEventListener("click", () => {
	changeLink("/cgi");
});