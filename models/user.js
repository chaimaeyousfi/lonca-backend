const mongoose = require('mongoose');

const userSchema = new mongoose.Schema({
    username: { type: String, required: true, unique: true },
    email: { type: String, required: true, unique: true, lowercase: true, match: /.+\@.+\..+/ },
    password: { type: String, required: true, minlength: 6 },
    phoneNumber: { type: String, required: true },
    name: { type: String },
    address: { type: String },
    position: { type: String },
    dob: { type: Date },
    profileImage: { type: String, default: null },
}, { timestamps: true });


const User = mongoose.model('User', userSchema);

module.exports = User;
