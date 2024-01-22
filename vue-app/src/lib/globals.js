// Apply
function shakeRef(ref) {
    ref.value = true;
    setTimeout(() => {
        ref.value = false;
    }, 300);
}

export {
    shakeElement
}