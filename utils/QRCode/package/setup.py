import setuptools

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="homekit_qrcode", # Replace with your own username
    scripts = ['scripts/homekit_qrcode'],
    version="0.0.1",
    author="An00bIS47",
    author_email="author@example.com",
    description="Generate Homekit QR codes",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/pypa/sampleproject",
    packages=setuptools.find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "Operating System :: OS Independent",
    ],
    python_requires='>=3.6',
)