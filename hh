/*import 'dart:convert';
import 'dart:io';
import 'package:flutter/material.dart';
import 'package:image_picker/image_picker.dart';
import 'package:http/http.dart' as http;
import 'package:shared_preferences/shared_preferences.dart';

class ProfilePage extends StatefulWidget {
  @override
  _ProfilePageState createState() => _ProfilePageState();
}

class _ProfilePageState extends State<ProfilePage> {
  final TextEditingController _nameController = TextEditingController();
  final TextEditingController _positionController = TextEditingController();
  final TextEditingController _phoneController = TextEditingController();
  final TextEditingController _emailController = TextEditingController();
  final TextEditingController _dobController = TextEditingController();
  final TextEditingController _addressController = TextEditingController();
  File? _profileImage;
  String? _profileImageUrl;
  final ImagePicker _picker = ImagePicker();
  bool _isLoading = false;
  String? _currentEmail;

  @override
  void initState() {
    super.initState();
    _fetchUserProfile();
  }

  Future<String?> _getUserEmail() async {
    SharedPreferences prefs = await SharedPreferences.getInstance();
    return prefs.getString('user_email');
  }

  Future<void> _fetchUserProfile() async {
    setState(() {
      _isLoading = true;
    });
    final email = await _getUserEmail();
    if (email == null) {
      _showMessage("User is not logged in.");
      return;
    }

    try {
      final url =
          Uri.parse('http://192.168.1.104:3000/get-profile?email=$email');
      final response = await http.get(url);
      if (response.statusCode == 200) {
        final userData = json.decode(response.body);
        setState(() {
          _nameController.text = userData['name'] ?? '';
          _positionController.text = userData['position'] ?? '';
          _phoneController.text = userData['phoneNumber'] ?? '';
          _emailController.text = userData['email'] ?? '';
          _dobController.text = userData['dob'] ?? '';
          _addressController.text = userData['address'] ?? '';
          _profileImageUrl = userData['profileImage'];
          _currentEmail = userData['email'];
        });
      } else {
        _showMessage("Failed to load profile data.");
      }
    } catch (error) {
      _showMessage("An error occurred while fetching profile data.");
    } finally {
      setState(() {
        _isLoading = false;
      });
    }
  }

  Future<void> _pickImage() async {
    final pickedFile = await _picker.pickImage(source: ImageSource.gallery);
    if (pickedFile != null) {
      setState(() {
        _profileImage = File(pickedFile.path);
      });
    } else {
      _showMessage("No image selected.");
    }
  }

  Future<void> _saveProfile() async {
    if (_currentEmail == null || _currentEmail!.isEmpty) {
      _currentEmail = _emailController.text.trim().toLowerCase();
      if (_currentEmail == null || _currentEmail!.isEmpty) {
        _showMessage("Current email is required.");
        return;
      }
    }

    final updatedProfile = {
      "currentEmail": _currentEmail!,
      "newEmail": _emailController.text.trim().toLowerCase(),
      "name": _nameController.text.trim(),
      "position": _positionController.text.trim(),
      "phoneNumber": _phoneController.text.trim(),
      "dob": _dobController.text.trim(),
      "address": _addressController.text.trim(),
    };

    final uri = Uri.parse('http://192.168.1.104:3000/update-profile');
    try {
      setState(() {
        _isLoading = true; // Start loading
      });
      final request = http.MultipartRequest('PUT', uri);

      if (_profileImage != null) {
        var picture = await http.MultipartFile.fromPath('profileImage',
            _profileImage!.path); // Match this field with Multer config
        request.files.add(picture);
      }

      request.fields.addAll(updatedProfile);

      final response = await request.send();

      if (response.statusCode == 200) {
        SharedPreferences prefs = await SharedPreferences.getInstance();
        await prefs.setString('user_email', updatedProfile['newEmail']!);
        _showMessage("Profile updated successfully!");
      } else {
        _showMessage("Failed to update profile. Please try again.");
      }
    } catch (error) {
      _showMessage("An error occurred while updating the profile.");
    } finally {
      setState(() {
        _isLoading = false; // Stop loading
      });
    }
  }

  void _showMessage(String message) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(content: Text(message)),
    );
  }

  int _selectedIndex = 1; // Profile is in the center position

  void _onItemTapped(int index) {
    setState(() {
      _selectedIndex = index;
    });

    switch (index) {
      case 0:
        Navigator.pushReplacementNamed(context, '/home');
        break;
      case 1:
        break;
      case 2:
        Navigator.pushReplacementNamed(context, '/profile');
        break;
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.white,
      body: SafeArea(
        child: LayoutBuilder(
          builder: (context, constraints) {
            return SingleChildScrollView(
              padding: EdgeInsets.only(
                  bottom: MediaQuery.of(context).viewInsets.bottom),
              child: ConstrainedBox(
                constraints: BoxConstraints(minHeight: constraints.maxHeight),
                child: Padding(
                  padding: const EdgeInsets.all(15.0),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.center,
                    children: [
                      Row(
                        mainAxisAlignment: MainAxisAlignment.spaceBetween,
                        children: [
                          IconButton(
                            icon: Icon(Icons.arrow_back, color: Colors.black),
                            onPressed: () {
                              Navigator.pop(context);
                            },
                          ),
                          Text(
                            'Your Profile',
                            style: TextStyle(
                              fontSize: 22,
                              fontWeight: FontWeight.bold,
                            ),
                          ),
                          SizedBox(width: 48),
                        ],
                      ),
                      SizedBox(height: 5),
                      Center(
                        child: Stack(
                          children: [
                            CircleAvatar(
                              radius: 50,
                              backgroundColor: Colors.grey[300],
                              backgroundImage: _profileImage != null
                                  ? FileImage(_profileImage!)
                                  : (_profileImageUrl?.isNotEmpty ?? false
                                          ? NetworkImage(_profileImageUrl!)
                                          : AssetImage(
                                              'assets/default-profile.png'))
                                      as ImageProvider,
                            ),
                            Positioned(
                              bottom: 0,
                              right: 0,
                              child: GestureDetector(
                                onTap: _pickImage,
                                child: CircleAvatar(
                                  radius: 16,
                                  backgroundColor: Colors.black,
                                  child: Icon(Icons.edit,
                                      color: Colors.white, size: 16),
                                ),
                              ),
                            ),
                          ],
                        ),
                      ),
                      SizedBox(height: 10),
                      _buildTitleAndTextField(
                          'Name', _nameController, 'Your name'),
                      SizedBox(height: 3),
                      _buildTitleAndTextField(
                          'Position', _positionController, 'Your position'),
                      SizedBox(height: 3),
                      _buildTitleAndTextField('Phone Number', _phoneController,
                          'Your phone number'),
                      SizedBox(height: 3),
                      _buildTitleAndTextField(
                          'Email', _emailController, 'Your email'),
                      SizedBox(height: 3),
                      _buildTitleAndTextField(
                          'Birthday', _dobController, 'YYYY-MM-DD'),
                      SizedBox(height: 3),
                      _buildTitleAndTextField(
                          'Address', _addressController, 'Your address'),
                      SizedBox(height: 15),
                      Center(
                          child: SizedBox(
                        width: double.infinity,
                        height: 45,
                        child: ElevatedButton(
                          style: ElevatedButton.styleFrom(
                            backgroundColor: Colors.black,
                            shape: RoundedRectangleBorder(
                              borderRadius: BorderRadius.circular(30),
                            ),
                          ),
                          onPressed: _isLoading
                              ? null
                              : _saveProfile, // Disable button while loading
                          child: _isLoading
                              ? CircularProgressIndicator(
                                  color: Colors.white,
                                )
                              : Text(
                                  'Update Profile',
                                  style: TextStyle(
                                    fontSize: 16,
                                    color: Colors.white,
                                  ),
                                ),
                        ),
                      ))
                    ],
                  ),
                ),
              ),
            );
          },
        ),
      ),
      bottomNavigationBar: BottomNavigationBar(
        items: <BottomNavigationBarItem>[
          BottomNavigationBarItem(
              icon: Icon(Icons.home, size: 24), label: 'Home'),
          BottomNavigationBarItem(
            icon: Container(
              decoration:
                  BoxDecoration(color: Colors.black, shape: BoxShape.circle),
              padding: EdgeInsets.all(4),
              child: Icon(Icons.add, size: 28, color: Colors.grey),
            ),
            label: '',
          ),
          BottomNavigationBarItem(
              icon: Icon(Icons.person, size: 24), label: 'Profile'),
        ],
        currentIndex: _selectedIndex,
        onTap: _onItemTapped,
        selectedItemColor: Colors.black,
        unselectedItemColor: Colors.grey,
        showSelectedLabels: true,
        showUnselectedLabels: true,
        iconSize: 24,
        selectedFontSize: 10, // Smaller font size for selected label
        unselectedFontSize: 10,
      ),
    );
  }

  Widget _buildTitleAndTextField(
      String title, TextEditingController controller, String hintText) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          title,
          style: TextStyle(
            fontSize: 14,
            color: Colors.grey[700],
          ),
        ),
        SizedBox(height: 5),
        _buildTextField(controller, hintText),
      ],
    );
  }

  Widget _buildTextField(TextEditingController controller, String hintText) {
    return TextFormField(
      controller: controller,
      decoration: InputDecoration(
        filled: true,
        fillColor: Colors.white,
        enabledBorder: OutlineInputBorder(
          borderRadius: BorderRadius.circular(12),
          borderSide: BorderSide(
            color: Colors.grey[300]!,
            width: 1,
          ),
        ),
        focusedBorder: OutlineInputBorder(
          borderRadius: BorderRadius.circular(12),
          borderSide: BorderSide(
            color: Colors.black,
            width: 1.5,
          ),
        ),
        contentPadding: EdgeInsets.symmetric(vertical: 10, horizontal: 15),
        hintText: hintText,
        hintStyle: TextStyle(
          color: Colors.grey.withOpacity(0.5),
          fontWeight: FontWeight.normal,
        ),
      ),
    );
  }
}


*/