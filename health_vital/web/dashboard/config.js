const firebaseConfig = {
    apiKey: "AIzaSyDM9C82uPDtmPktWPE7yOFnFHBkVrXcgJs",
    databaseURL: "https://wearable-health-device-default-rtdb.firebaseio.com/"
};

firebase.initializeApp(firebaseConfig);
const db = firebase.database();