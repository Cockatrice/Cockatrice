document.addEventListener("DOMContentLoaded", () => {
    document.querySelectorAll(".memdoc").forEach(memdoc => {
        let callContent = null, callHeader = null;
        let callerContent = null, callerHeader = null;

        memdoc.querySelectorAll("div.dynheader").forEach(header => {
            const text = (header.textContent || "").trim().toLowerCase();
            let content = header.nextElementSibling;
            let tries = 0;
            while (content && !content.classList.contains("dyncontent") && tries < 8) {
                content = content.nextElementSibling;
                tries++;
            }
            if (!content) return;

            if (text.includes("caller")) {
                callerContent = content;
                callerHeader = header;
            } else if (text.includes("call graph") || text.includes("callgraph") || text.includes("call graph for")) {
                callContent = content;
                callHeader = header;
            } else if (text.includes("call")) {
                if (!callContent) {
                    callContent = content;
                    callHeader = header;
                }
            }
        });

        if (!callContent && !callerContent) return;
        if (memdoc.querySelector(".graph-toggle")) return;

        const toggle = document.createElement("div");
        toggle.className = "graph-toggle";

        const callerBtn = document.createElement("button");
        callerBtn.type = "button";
        callerBtn.textContent = "Caller Graph";
        const callBtn = document.createElement("button");
        callBtn.type = "button";
        callBtn.textContent = "Call Graph";

        toggle.appendChild(callerBtn);
        toggle.appendChild(callBtn);

        const firstHeader = memdoc.querySelector("div.dynheader");
        memdoc.insertBefore(toggle, firstHeader || memdoc.firstChild);

        // hide everything initially
        if (callerContent) {
            callerContent.style.display = "none";
            callerHeader.style.display = "none";
        }
        if (callContent) {
            callContent.style.display = "none";
            callHeader.style.display = "none";
        }

        // disable missing buttons
        if (!callerContent) {
            callerBtn.disabled = true;
            callerBtn.classList.add("disabled");
        }
        if (!callContent) {
            callBtn.disabled = true;
            callBtn.classList.add("disabled");
        }

        // track current state
        let current = null; // "caller", "call", "both", null=hidden

        function setActive(type) {
            if (type === "caller") {
                if (current === "caller") { // hide it
                    if (callerContent) {
                        callerContent.style.display = "none";
                        callerHeader.style.display = "none";
                    }
                    current = null;
                } else if (current === "call") { // show both
                    if (callerContent) {
                        callerContent.style.display = "block";
                        callerHeader.style.display = "block";
                    }
                    current = "both";
                } else if (current === "both") { // hide caller only → call only
                    if (callerContent) {
                        callerContent.style.display = "none";
                        callerHeader.style.display = "none";
                    }
                    current = "call";
                } else { // nothing visible → show caller
                    if (callerContent) {
                        callerContent.style.display = "block";
                        callerHeader.style.display = "block";
                    }
                    if (callContent) {
                        callContent.style.display = "none";
                        callHeader.style.display = "none";
                    }
                    current = "caller";
                }
            } else if (type === "call") {
                if (current === "call") { // hide it
                    if (callContent) {
                        callContent.style.display = "none";
                        callHeader.style.display = "none";
                    }
                    current = null;
                } else if (current === "caller") { // show both
                    if (callContent) {
                        callContent.style.display = "block";
                        callHeader.style.display = "block";
                    }
                    current = "both";
                } else if (current === "both") { // hide call only → caller only
                    if (callContent) {
                        callContent.style.display = "none";
                        callHeader.style.display = "none";
                    }
                    current = "caller";
                } else { // nothing visible → show call only
                    if (callContent) {
                        callContent.style.display = "block";
                        callHeader.style.display = "block";
                    }
                    if (callerContent) {
                        callerContent.style.display = "none";
                        callerHeader.style.display = "none";
                    }
                    current = "call";
                }
            }

            // update button styles
            callerBtn.classList.toggle("active", current === "caller" || current === "both");
            callBtn.classList.toggle("active", current === "call" || current === "both");
        }

        callerBtn.addEventListener("click", () => {
            if (!callerBtn.disabled) setActive("caller");
        });
        callBtn.addEventListener("click", () => {
            if (!callBtn.disabled) setActive("call");
        });
    });
});
