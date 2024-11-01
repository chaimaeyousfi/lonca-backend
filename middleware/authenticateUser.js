const jwt = require('jsonwebtoken');

function authenticateUser(req, res, next) {
    const token = req.headers['authorization']?.split(' ')[1]; // Get token from headers
    if (!token) return res.status(401).send('Access denied.');

    jwt.verify(token, process.env.JWT_SECRET, (err, user) => {
        if (err) return res.status(403).send('Invalid token.');
        req.user = user; // Attach user data to request
        next();
    });
}

module.exports = authenticateUser;