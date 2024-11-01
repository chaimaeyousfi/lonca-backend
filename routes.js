const express = require('express');
const router = express.Router();
const bcrypt = require('bcryptjs');
const nodemailer = require('nodemailer');
const User = require('./models/user');
const cors = require('cors');
const multer = require('multer');
const multerS3 = require('multer-s3');
const { S3Client } = require('@aws-sdk/client-s3');
const session = require('express-session');
const MongoStore = require('connect-mongo'); 
const mongoose = require('mongoose');


router.use(cors());

const s3 = new S3Client({
    region: 'us-east-1',
    credentials: {
        accessKeyId: 'AKIA4IM3G6FH6E5LVCVL',
        secretAccessKey: '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',
    },
});

const upload = multer({
    storage: multerS3({
        s3: s3,
        bucket: 'minimalistapp',
        key: function (req, file, cb) {
            cb(null, `profile-images/${Date.now().toString()}-${file.originalname}`); 
        },
    }),
});

const sessionMiddleware = session({
    secret: 'secretKey',
    resave: false,
    saveUninitialized: false,
    store: MongoStore.create({ mongoUrl: 'mongodb+srv://cychaimaeyousfi:cychaimaeyousfi@minimalist.4t6ry.mongodb.net/' }),
});
router.post('/upload', upload.single('file'), async (req, res) => {
    console.log("Upload route hit");
    try {
        if (!req.file) {
            return res.status(400).json({ message: 'No file uploaded' });
        }
        res.json({ message: 'File uploaded successfully', url: req.file.location });
    } catch (error) {
        console.error("Error during file upload:", error);
        res.status(500).json({ message: 'Internal server error', error });
    }
});

router.post('/signup', async (req, res) => {
    const { username, email, password, phoneNumber } = req.body;

    if (!username || !email || !password || !phoneNumber) {
        return res.status(400).json({ message: 'Please fill all fields' });
    }

    const existingUser = await User.findOne({ email });
    if (existingUser) {
        return res.status(400).json({ message: 'User already exists' });
    }

    const hashedPassword = await bcrypt.hash(password, 10);

    const newUser = new User({
        username,
        email,
        password: hashedPassword,
        phoneNumber,
    });

    try {
        await newUser.save();
        res.status(200).json({ message: 'Registration successful' });
    } catch (error) {
        console.error('Error registering user:', error);
        res.status(500).json({ message: 'Error registering user' });
    }
});

router.post('/signin', async (req, res) => {
    const { email, password } = req.body;

    if (!email || !password) {
        return res.status(400).json({ message: 'Email and password are required' });
    }

    const user = await User.findOne({ email: email.toLowerCase() });
    if (!user) {
        return res.status(401).json({ message: 'Invalid email or password' });
    }

    const isMatch = await bcrypt.compare(password, user.password);
    if (!isMatch) {
        return res.status(401).json({ message: 'Invalid email or password' });
    }

    res.status(200).json({ message: 'Sign-in successful', user });
});

const transporter = nodemailer.createTransport({
    service: 'gmail',
    auth: {
        user: 'cy.chaimaeyousfi@gmail.com',
        pass: 'ovfd cduw ydnz yigi', 
    },
});

router.post('/forgot-password', async (req, res) => {
    const { email } = req.body;

    if (!email) {
        return res.status(400).json({ message: 'Email is required' });
    }

    const user = await User.findOne({ email });
    if (!user) {
        return res.status(404).json({ message: 'User not found' });
    }

    const mailOptions = {
        from: 'cy.chaimaeyousfi@gmail.com',
        to: email,
        subject: 'Password Reset Request',
        text: 'Click the link below to reset your password:\n\nhttp://your-reset-link.com',
    };

    transporter.sendMail(mailOptions, (error, info) => {
        if (error) {
            console.error('Error sending email:', error);
            return res.status(500).json({ message: 'Error sending email' });
        }
        res.status(200).json({ message: 'Verification link sent to your email' });
    });
});

router.get('/get-profile', async (req, res) => {
    const { email } = req.query;

    if (!email) {
        return res.status(400).json({ message: 'Email is required' });
    }

    try {
        const user = await User.findOne({ email: email.toLowerCase() });

        if (!user) {
            return res.status(404).json({ message: 'User not found' });
        }

      
        res.status(200).json({
            name: user.name,
            position: user.position,
            phoneNumber: user.phoneNumber,
            email: user.email,
            dob: user.dob,
            address: user.address,
            profileImage: user.profileImage,
        });
    } catch (error) {
        console.error('Error fetching user profile:', error); 
        res.status(500).json({ message: 'Internal server error' });
    }
});


router.put('/update-profile', upload.single('profileImage'), async (req, res) => {
    const { currentEmail, newEmail, name, position, phoneNumber, dob, address } = req.body;

    if (!currentEmail) {
        return res.status(400).json({ message: 'Current email is required' });
    }

    try {
        
        const user = await User.findOne({ email: currentEmail.toLowerCase() });
        if (!user) {
            return res.status(404).json({ message: 'User not found' });
        }

        
        const updatedData = {
            ...(name && { name }),
            ...(position && { position }),
            ...(phoneNumber && { phoneNumber }),
            ...(dob && { dob }),
            ...(address && { address }),
            ...(req.file && { profileImage: req.file.location })  
        };

        if (newEmail && newEmail !== currentEmail) {
            const existingUser = await User.findOne({ email: newEmail.toLowerCase() });
            if (existingUser) {
                return res.status(400).json({ message: 'New email is already in use' });
            }
            updatedData.email = newEmail.toLowerCase();  
        }

        
        const updatedUser = await User.findOneAndUpdate(
            { email: currentEmail.toLowerCase() },
            { $set: updatedData },
            { new: true }  
        );

        res.status(200).json({
            message: 'Profile updated successfully',
            user: updatedUser
        });
    } catch (error) {
        console.error('Error updating user profile:', error);
        res.status(500).json({ message: 'Internal server error', error: error.message });
    }
});



async function clearUserSessions(user) {
    const sessionCollection = mongoose.connection.collection('sessions'); 

    
    try {
        const result = await sessionCollection.deleteMany({ userId: user._id });
        console.log(`Invalidated ${result.deletedCount} sessions for user ID ${user._id}.`);
    } catch (error) {
        console.error("Error invalidating user sessions:", error);
    }
}


router.put('/change-password', async (req, res) => {
    const { email, currentPassword, newPassword, logOutOtherDevices } = req.body;

    if (!email || !currentPassword || !newPassword) {
        return res.status(400).json({ message: 'Email, current password, and new password are required.' });
    }

    try {
        console.log("Received change-password request");

        const user = await User.findOne({ email: email.toLowerCase() });
        if (!user) {
            console.log("User not found");
            return res.status(404).json({ message: 'User not found.' });
        }

        const isMatch = await bcrypt.compare(currentPassword, user.password);
        if (!isMatch) {
            console.log("Current password is incorrect");
            return res.status(401).json({ message: 'Current password is incorrect.' });
        }

        const hashedNewPassword = await bcrypt.hash(newPassword, 10);
        user.password = hashedNewPassword;

        await user.save();

       
        if (logOutOtherDevices) {
            console.log("Logging out from other devices");
            await clearUserSessions(user);
        }

        console.log("Password updated successfully");
        res.status(200).json({ message: 'Password changed successfully.' });
    } catch (error) {
        console.error("Error changing password:", error);
        res.status(500).json({ message: 'Internal server error' });
    }
});
module.exports = router;
