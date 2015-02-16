//
//  ViewController.swift
//  LoLUpdater
//
//  Created by David Knaack on 31.12.14.
//
//

import Cocoa
import Alamofire

class ViewController: NSViewController {

	override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
		
		let userDefaults = NSUserDefaults.standardUserDefaults()
		if let path = userDefaults.valueForKey("defaultPath") as? String {
			pathInput.stringValue = path
		}
		
    }

    
    override var representedObject: AnyObject? {
        didSet {
        // Update the view, if already loaded.
        }
    } 

    @IBOutlet weak var progressBar: NSProgressIndicator!
    @IBOutlet weak var installOrRemove: NSMatrix!
    @IBOutlet weak var pathInput: NSTextField!
    @IBOutlet weak var findLoLButton: NSButton!

    @IBAction func findLoLButtonClick(sender: NSButton) {
		let openPanel = NSOpenPanel()
		
		
		openPanel.allowedFileTypes = ["app"]
		openPanel.message = "Please select the League of Legends.app"
		
		openPanel.allowsMultipleSelection = false
		openPanel.canChooseDirectories = false
		openPanel.canCreateDirectories = false
		openPanel.canChooseFiles = true
		openPanel.beginSheetModalForWindow(self.view.window!, completionHandler: { (result) -> Void in
			if result == NSFileHandlingPanelOKButton {
				self.pathInput.stringValue = openPanel.URL!.path!
			}
		})
		
    }
    @IBAction func patchButtonClick(sender: NSButton) {
		let fm = NSFileManager.defaultManager()
		
		let path = pathInput.stringValue.isEmpty ? pathInput.stringValue : pathInput.placeholderString!
		// TODO: proper error reporting and validation
		if !path.hasSuffix("app")
		{
			return
		}
		
		setDefaultPath(path)
		
		sender.enabled = false
        installOrRemove.enabled = false
        pathInput.enabled = false
        findLoLButton.enabled = false
        
        sender.title = "Working…"
        progressBar.hidden = false
		
		if installOrRemove.stringValue == "1"
		{
			install(path)
		} else {
			remove(path)
		}
		fm
    }
	
	func install(path: String) {
		let tempDir = tempDirectory()
		NSLog(tempDir)
		Alamofire.download(.GET, "http://labsdownload.adobe.com/pub/labs/flashruntimes/air/air17_mac.dmg", { (temporaryURL, response) in
			return NSURL(fileURLWithPath: "\(tempDir)/air.dmg")!
		}).response { (request, response, _, error) in
			self.mount("\(tempDir)/air.dmg")
			
			//
			// Replace Stuff
			//
			
			
			self.unmount("/Volumes/Adobe Air/")
		}
		
		Alamofire.download(.GET, "http://developer.download.nvidia.com/cg/Cg_3.1/Cg-3.1_April2012.dmg", { (temporaryURL, response) in
			return NSURL(fileURLWithPath: "\(tempDir)/cg.dmg")!
		}).response { (request, response, _, error) in
			self.mount("\(tempDir)/air.dmg")
			
			//
			// Replace Stuff
			//
			
			
			self.unmount("/Volumes/cg-3.1.0013")
		}
		

		
	}
	
	func remove (path: String)
	{
	
	}
	

	func copy(from: String, to: String, err: NSErrorPointer = nil) {
		let fm = NSFileManager.defaultManager()
		fm.copyItemAtPath(from, toPath: to, error: err)
	}
	

	func replace(from: String, backupPath: String, to: String...) {
		// Create Backup
		copy(to[0], to: backupPath)
		let fm = NSFileManager.defaultManager()
		
		for path in to {
			// TODO: Replace file
		}
		
	}
	
	func setDefaultPath(path: String) {
		var userDefaults = NSUserDefaults.standardUserDefaults()
		userDefaults.setValue(path, forKey: "defaultPath")
		userDefaults.synchronize()
	}
	
	func highestVersionNumber(path: String) -> String? {
		if let dirURL = NSURL(fileURLWithPath: path) {
			let keys = [NSURLIsDirectoryKey, NSURLLocalizedNameKey]
			let fm = NSFileManager.defaultManager()
			
			let enumerator = fm.enumeratorAtURL(
				dirURL,
				includingPropertiesForKeys: keys,
				options: (NSDirectoryEnumerationOptions.SkipsPackageDescendants |
					NSDirectoryEnumerationOptions.SkipsSubdirectoryDescendants |
					NSDirectoryEnumerationOptions.SkipsHiddenFiles),
				errorHandler: {(url, error) -> Bool in
					return true
				}
			)
			
			while let element = enumerator?.nextObject() as? NSURL {
				var getter: AnyObject?
				element.getResourceValue(&getter, forKey: NSURLIsDirectoryKey, error: nil)
				let isDirectory = getter! as Bool
				
				if isDirectory {
					element.getResourceValue(&getter, forKey: NSURLLocalizedNameKey, error: nil)
					return getter? as? String
				}
			}
		}
		return nil
	}
	
	// http://stackoverflow.com/a/24290234/1183431
	func tempDirectory()->String! {
		let tempDirectoryTemplate = "\(NSTemporaryDirectory())XXXXX"
		var tempDirectoryTemplateCString = tempDirectoryTemplate.fileSystemRepresentation()
		let result = mkdtemp(&tempDirectoryTemplateCString)
		if result == nil {
			return nil
		}
		let fm = NSFileManager.defaultManager()
		let tempDirectoryPath = fm.stringWithFileSystemRepresentation(result, length: Int(strlen(result)))
		return tempDirectoryPath
	}
	
	func mount(path: String) {
		let task = NSTask()
		task.launchPath = "/usr/bin/hdiutil"
		// TODO: Check for errors
		task.arguments = ["attach", "-nobrowse", path]
		
		task.launch()
		task.waitUntilExit()
		
		
	}
	
	func unmount(volume: String) {
		let task = NSTask()
		task.launchPath = "/usr/bin/hdiutil"
		// TODO: Check for errors
		task.arguments = ["detach", volume]
		
		task.launch()
		task.waitUntilExit()
		
		
	}

}

