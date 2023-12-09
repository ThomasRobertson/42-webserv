function changeLink(link) {
	window.location.href = link;
}

const errors_button = document.getElementById("errors_button");
const auth_button = document.getElementById("auth_button");

errors_button.addEventListener("click", () => {
	changeLink("/errors");
});

auth_button.addEventListener("click", () => {
	changeLink("/auth");
});

cookies_button.addEventListener("click", () => {
	changeLink("/cgi");
});

cgi_alt_button.addEventListener("click", () => {
	changeLink("/cgi_alt");
});

upload_button.addEventListener("click", () => {
	changeLink("/form");
});

invalid_upload_button.addEventListener("click", () => {
	changeLink("/post_body");
});

invalid_list_dir_button.addEventListener("click", () => {
	changeLink("/no_list_dir");
});

redirect_button.addEventListener("click", () => {
	changeLink("/cgi_redirect");
});

redirect_google_button.addEventListener("click", () => {
	changeLink("/google");
});