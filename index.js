const express = require('express');
const mongoose = require('mongoose');
const userRoutes = require('./routes');
const cors = require('cors');
const session = require('express-session'); 
const MongoStore = require('connect-mongo'); 

const app = express();
app.use(cors());
app.use(express.json());


mongoose.connect('mongodb+srv://cychaimaeyousfi:cychaimaeyousfi@minimalist.4t6ry.mongodb.net/', {
    useNewUrlParser: true,
    useUnifiedTopology: true,
})
    .then(() => console.log('Connected to MongoDB'))
    .catch((err) => console.error('Could not connect to MongoDB...', err));


app.use(session({
    secret: 'secretKey', 
    resave: false,
    saveUninitialized: false,
    store: MongoStore.create({ mongoUrl: 'mongodb+srv://cychaimaeyousfi:cychaimaeyousfi@minimalist.4t6ry.mongodb.net/', collectionName: 'sessions' }), // Ensure this points to your MongoDB instance
    cookie: { secure: false } 
}));

app.use('/', userRoutes);

const port = process.env.PORT || 3000;
app.listen(port, () => {
    console.log(`Server running on port ${port}`);
});
